# ETL 타입 제거 및 DataType 기반 리팩토링

## 🎯 목표

ETL(Extract, Transform, Load) 타입 분류를 제거하고, **Input/Output DataType 기반**의 더 유연한 플러그인 시스템으로 전환

## 📋 변경 사항

### 1. Plugin API 개선 ([plugin_api.h](include/uniconv/plugin_api.h))

#### 추가된 DataType

```c
typedef enum {
    UNICONV_DATA_FILE = 0,      // Generic file (path-based)
    UNICONV_DATA_IMAGE = 1,     // Image data
    UNICONV_DATA_VIDEO = 2,     // Video data
    UNICONV_DATA_AUDIO = 3,     // Audio data
    UNICONV_DATA_TEXT = 4,      // Text data
    UNICONV_DATA_JSON = 5,      // Structured JSON data
    UNICONV_DATA_BINARY = 6,    // Binary blob
    UNICONV_DATA_STREAM = 7     // Stream data
} UniconvDataType;
```

#### 업데이트된 PluginInfo

```c
typedef struct {
    const char* name;
    const char* scope;
    UniconvETLType etl;         // DEPRECATED: 하위 호환용
    const char* version;
    const char* description;
    const char** targets;
    const char** input_formats;

    // NEW: 입출력 타입 정보
    UniconvDataType* input_types;   // NULL-terminated array
    UniconvDataType* output_types;  // NULL-terminated array

    int api_version;            // 2로 증가
} UniconvPluginInfo;
```

### 2. Core Types 업데이트 ([types.h](src/core/types.h))

- `DataType` enum 추가 (C++ 버전)
- `ETLType` → DEPRECATED 표시
- `PluginInfo` 구조체에 `input_types`, `output_types` 필드 추가

### 3. PluginManager 개선 ([plugin_manager.h](src/core/plugin_manager.h))

#### 새로운 API

```cpp
// ETL 타입 없이 target만으로 플러그인 찾기
plugins::IPlugin* find_plugin(
    const std::string& target,
    const std::optional<std::string>& explicit_plugin = std::nullopt,
    ETLType etl = ETLType::Transform  // DEPRECATED
);

// 입력 포맷과 target으로 플러그인 찾기
plugins::IPlugin* find_plugin_for_input(
    const std::string& input_format,
    const std::string& target,
    ETLType etl = ETLType::Transform  // DEPRECATED
);

// 플러그인 연결 가능 여부 체크 (NEW!)
bool can_connect(const PluginInfo& from, const PluginInfo& to) const;
```

#### 타입 호환성 체크

```cpp
bool PluginManager::can_connect(const PluginInfo& from, const PluginInfo& to) const {
    // If either plugin doesn't specify types, assume File type (always compatible)
    if (from.output_types.empty() || to.input_types.empty()) {
        return true;
    }

    // Check if any output type from 'from' matches any input type in 'to'
    for (const auto& out_type : from.output_types) {
        for (const auto& in_type : to.input_types) {
            if (out_type == in_type ||
                out_type == DataType::File ||
                in_type == DataType::File) {
                return true;
            }
        }
    }

    return false;
}
```

### 4. PipelineExecutor 단순화 ([pipeline_executor.cpp](src/core/pipeline_executor.cpp))

- **제거**: `determine_etl_type()` 함수 완전 삭제
- ETL 타입 추론 불필요 - 플러그인이 알아서 처리

### 5. 플러그인 예제 업데이트 ([video-to-gif](plugins/examples/cpp/video-to-gif/video_to_gif.cpp))

```cpp
// 기존
static UniconvPluginInfo plugin_info = {
    .name = "video-convert",
    .scope = "video-convert",
    .etl = UNICONV_ETL_TRANSFORM,  // 이게 왜 필요했나?
    .version = "1.0.0",
    .description = "Convert video to GIF using libav",
    .targets = targets,
    .input_formats = input_formats,
    .api_version = UNICONV_API_VERSION
};

// 개선 후
static UniconvDataType input_types[] = {
    UNICONV_DATA_VIDEO,
    UNICONV_DATA_FILE,
    (UniconvDataType)0
};
static UniconvDataType output_types[] = {
    UNICONV_DATA_IMAGE,
    (UniconvDataType)0
};

static UniconvPluginInfo plugin_info = {
    .name = "video-convert",
    .scope = "video-convert",
    .etl = UNICONV_ETL_TRANSFORM,  // DEPRECATED: 호환성 유지
    .version = "1.0.0",
    .description = "Convert video to GIF using libav",
    .targets = targets,
    .input_formats = input_formats,
    .input_types = input_types,    // NEW: Video 또는 File 입력
    .output_types = output_types,  // NEW: Image 출력
    .api_version = 2
};
```

## ✅ 장점

### 1. 유연한 파이프라인

```bash
# ETL 타입 구분 없이 자유로운 조합
uniconv "video.mp4 | audio | transcript | summary | pdf"
#         Video → Audio → Text → Text → Document
#         각 단계는 input/output 타입만 맞으면 자동 연결!
```

### 2. 명확한 타입 체크

```cpp
// 플러그인 A의 output이 플러그인 B의 input과 호환되는지 자동 체크
if (plugin_manager.can_connect(pluginA.info(), pluginB.info())) {
    // 연결 가능!
}
```

### 3. 불필요한 분류 제거

```
ETL 타입의 문제:
❌ video-to-gif는 Transform? Extract도 될 수 있는데?
❌ OCR은 Extract? 텍스트 파일 만들면 Transform?
❌ 경계가 애매한 플러그인은?

DataType의 해결:
✅ 입력: Video/File, 출력: Image → 명확!
✅ 입력: Image, 출력: Text → 명확!
✅ 타입만 맞으면 어떤 조합도 가능!
```

### 4. 하위 호환성 유지

- 기존 ETL 타입 관련 코드는 DEPRECATED 표시만 하고 유지
- 기존 플러그인도 그대로 작동
- 점진적 마이그레이션 가능

## 📊 테스트 결과

```
[==========] Running 63 tests from 6 test suites.
...
[  PASSED  ] 63 tests.
```

모든 기존 테스트 통과! ✅

## 🚀 다음 단계 (Optional)

1. **Phase 2**: 내장 플러그인들 DataType 추가
2. **Phase 3**: ETL 타입 완전 제거 (breaking change)
3. **Phase 4**: 스트림 기반 파이프라인 (메모리 효율)

## 📝 마이그레이션 가이드

### 기존 플러그인 업데이트하기

```c
// 1. input_types, output_types 배열 정의
static UniconvDataType input_types[] = {
    UNICONV_DATA_IMAGE,  // 받을 수 있는 타입들
    UNICONV_DATA_FILE,
    (UniconvDataType)0   // NULL terminator
};

static UniconvDataType output_types[] = {
    UNICONV_DATA_IMAGE,  // 출력하는 타입들
    (UniconvDataType)0
};

// 2. PluginInfo에 추가
static UniconvPluginInfo plugin_info = {
    // ... 기존 필드들 ...
    .input_types = input_types,
    .output_types = output_types,
    .api_version = 2  // 중요!
};
```

## 💡 핵심 개념

**"플러그인은 ETL 타입이 아니라, 데이터 변환기다"**

- 입력 타입 → 출력 타입 변환
- 타입만 맞으면 자유롭게 조합 가능
- 파이프라인에서 자동 타입 체크

---

**리팩토링 완료일**: 2026-02-03
**API Version**: 1 → 2 → 3
**Breaking Changes**: 없음 (하위 호환성 유지)
