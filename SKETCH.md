# uniconv - Universal Converter & Content Intelligence Tool

## 프로젝트 요약

컨텍스트 메뉴 통합과 AI 기반 콘텐츠 이해 기능을 갖춘 크로스 플랫폼 파일 변환 도구.
"우클릭 한 방"의 편리함과 파워유저를 위한 CLI 확장성을 동시에 제공.

**단순 포맷 변환을 넘어서:**

- 콘텐츠를 **변환**하고 (Transform)
- 원하는 것을 **추출**하고 (Extract)
- 결과를 **적재**하는 (Load)

**ETL 개념 기반 파이프라인**으로 복잡한 작업도 간단하게.

---

## 1. 핵심 가치

### 1.1 차별화 포인트 (vs ImageMagick, FFmpeg)

| 기능               | ImageMagick/FFmpeg | uniconv                           |
| ------------------ | ------------------ | --------------------------------- |
| 파이프라인 문법    | ❌                 | ✅ `"source \| target \| target"` |
| 프리셋 시스템      | ❌                 | ✅ 저장/불러오기                |
| Watch 모드         | ❌                 | ✅ 폴더 감시 자동 변환          |
| JSON 출력          | ❌                 | ✅ LLM/자동화 친화적            |
| 타겟 용량 지정     | ❌                 | ✅ `--target-size 25MB`         |
| AI 이미지 처리     | ❌                 | ✅ 배경 제거, 자동 보정 등      |
| 컨텍스트 메뉴 통합 | 수동 스크립트      | ✅ 자동 설치                    |
| Interactive 모드   | ❌                 | ✅ 초보자 친화적                |
| 콘텐츠 추출        | ❌                 | ✅ 얼굴, 텍스트, 표, 장면 등    |
| 의미 기반 검색     | ❌                 | ✅ "해변 사진 찾아줘"           |
| 구조화 파싱        | ❌                 | ✅ 영수증, 명함 → JSON          |
| 플러그인 시스템    | ❌                 | ✅ 다중 언어 지원               |

### 1.2 타겟 사용자

- **일반 사용자**: 컨텍스트 메뉴로 간편 변환
- **파워 유저**: CLI 파이프라인 + 프리셋 + 자동화
- **개발자**: JSON 출력, 플러그인 개발 (다중 언어)
- **LLM/Agent**: 구조화된 I/O

---

## 2. 파이프라인 문법

### 2.1 기본 구조

```bash
uniconv [core옵션] <source> "<target>[@plugin] [옵션] | <target>[@plugin] [옵션] | ..."
```

**중요: 파이프라인(타겟 체인)은 반드시 따옴표로 감싸야 함** (쉘 `|` 연산자와 충돌 방지)

- `|` = 스테이지 구분
- `,` = 같은 스테이지 내 병렬 요소
- `tee` = 다음 스테이지 요소 개수만큼 복제하는 builtin
- `clipboard` = 결과를 시스템 클립보드에 복사
- `_` (passthrough) = 입력을 그대로 전달

### 2.2 기본 예시

```bash
# 단일 변환
uniconv photo.heic "jpg"
uniconv photo.heic "jpg --quality 90"
uniconv video.mov "mp4"

# 이미지 필터
uniconv photo.jpg "grayscale"
uniconv photo.jpg "ascii --width 80"

# 문서 변환
uniconv document.docx "pdf"
uniconv photo.jpg "pdf | docx"    # 다단계: 이미지 → PDF → DOCX

# 비디오 변환
uniconv video.mp4 "gif --width 320 --fps 15"
uniconv video.mp4 "mp4 --height 720"

# 클립보드로 복사
uniconv photo.jpg "png | clipboard"

# 출력 경로 지정
uniconv -o output.jpg photo.heic "jpg --quality 85"
```

### 2.3 플러그인 지정

```bash
# @로 플러그인 지정 (같은 타겟을 여러 플러그인이 지원할 때)
uniconv photo.heic "jpg@image-convert --quality 90"
uniconv video.mov "mp4@video-convert --crf 23"
```

### 2.4 tee를 이용한 분기

`tee`는 다음 스테이지 요소 개수만큼 현재 결과를 복제.

```bash
# 하나를 여러 포맷으로
uniconv photo.heic "tee | jpg, png, webp"

#   photo.heic → tee (3개 복제)
#                  → [0] jpg
#                  → [1] png
#                  → [2] webp

# 변환 후 분기
uniconv photo.heic "jpg | tee | grayscale, invert"

#   photo.heic → jpg → tee (2개 복제)
#                        → [0] grayscale
#                        → [1] invert
```

### 2.5 분기 후 각각 다른 처리

분기된 각 요소는 독립적인 파이프라인을 가질 수 있음.

```bash
uniconv photo.heic "tee | jpg --quality 90, png, webp --quality 80"

#   photo.heic → tee (3개 복제)
#                  → [0] jpg --quality 90
#                  → [1] png
#                  → [2] webp --quality 80

# 분기 후 계속 진행
uniconv photo.jpg "tee | grayscale | clipboard, png"

#   photo.jpg → tee (2개 복제)
#                 → [0] grayscale → clipboard
#                 → [1] png (파일 저장)
```

### 2.6 스테이지 요소 개수 규칙

| 현재 스테이지 | 다음 스테이지 | 가능 여부     |
| ------------- | ------------- | ------------- |
| 1개           | 1개           | ✅            |
| 1개 (tee)     | N개           | ✅            |
| N개           | N개           | ✅ (1:1 매핑) |
| N개           | M개 (N≠M)     | ❌            |

```bash
# ✅ 1 → 1 → 1
uniconv photo.heic "jpg | grayscale"

# ✅ 1 → tee → 3
uniconv photo.heic "tee | jpg, png, webp"

# ✅ 1 → 1 → tee → 2
uniconv photo.heic "jpg | tee | grayscale, invert"

# ❌ 1 → 2 (tee 없이 늘어남)
uniconv photo.heic "jpg, png"

# ❌ 2 → 3 (개수 불일치)
uniconv photo.heic "tee | jpg, png | grayscale, invert, negative"
```

### 2.7 Interactive 모드

```bash
# 파이프라인 없으면 interactive 진입
uniconv photo.heic

# 명시적 interactive
uniconv --interactive photo.heic
```

---

## 3. ETL 타겟 종류

### 3.1 세 가지 ETL 타입

모든 타겟은 ETL 중 하나에 속함:

| 타입          | 의미 | 예시 타겟                                   |
| ------------- | ---- | ------------------------------------------- |
| **Transform** | 변환 | jpg, png, mp4, gif, pdf, ...                |
| **Extract**   | 추출 | faces, audio, text, scenes, highlights, ... |
| **Load**      | 적재 | gdrive, s3, dropbox, notion, ...            |

타겟 이름으로 ETL 타입이 자동 결정됨 (플러그인 등록 시 지정).

### 3.2 Builtin 타겟

| 타겟              | 설명                                      |
| ----------------- | ----------------------------------------- |
| `tee`             | 다음 스테이지로 분기 (입력을 N개로 복제)  |
| `clipboard`       | 결과를 시스템 클립보드에 복사             |
| `_` (passthrough) | 입력을 그대로 전달 (분기 시 일부만 처리)  |

**clipboard 동작:**
- 이미지/텍스트 포맷: 콘텐츠를 클립보드에 직접 복사
- 기타 포맷: 파일 경로를 클립보드에 복사
- 기본적으로 파일 미생성 (`--save` 또는 `-o`로 파일 저장)

**passthrough 별칭:** `_`, `echo`, `bypass`, `pass`, `noop`

### 3.3 현재 사용 가능한 플러그인

| 플러그인         | 타입           | 지원 타겟                              |
| ---------------- | -------------- | -------------------------------------- |
| `image-convert`  | Native (C++)   | jpg, png, webp, gif, heic, bmp, tiff   |
| `video-convert`  | Native (C++)   | mp4, mov, avi, webm, mkv, gif          |
| `doc-convert`    | CLI (Python)   | pdf, docx, odt, xlsx, pptx, md, html   |
| `ascii`          | CLI (Python)   | ascii, ascii-art, text-art             |
| `image-filter`   | CLI (Python)   | grayscale, invert, negative            |

**설치:**
```bash
uniconv plugin install +essentials    # 모든 기본 플러그인
uniconv plugin install image-convert  # 개별 플러그인
```

---

## 4. 플러그인 시스템

### 4.1 설계 원칙

- **하나의 플러그인 = 하나의 ETL 타입**
- **하나의 플러그인 → 여러 타겟 지원 가능**
- **여러 플러그인 → 같은 타겟 지원 가능**
- **같은 스코프명 공유 가능** (예: ffmpeg.transform, ffmpeg.extract)

### 4.2 플러그인 식별

```
<스코프명>.<etl타입>
```

| 식별자              | 스코프명  | ETL       |
| ------------------- | --------- | --------- |
| `ffmpeg.transform`  | ffmpeg    | transform |
| `ffmpeg.extract`    | ffmpeg    | extract   |
| `ai-vision.extract` | ai-vision | extract   |
| `gdrive.load`       | gdrive    | load      |

파이프라인에서는 스코프명만 사용 (타겟으로 ETL 결정):

```bash
uniconv "video.mov | mp4@ffmpeg"      # → ffmpeg.transform
uniconv "video.mov | audio@ffmpeg"    # → ffmpeg.extract
```

### 4.3 플러그인 타입

#### Native 플러그인 (C/C++)

성능 크리티컬한 경우:

```cpp
// plugin.h
extern "C" {
    PluginInfo* uniconv_plugin_info();
    Result* uniconv_plugin_execute(Request* req);
    void uniconv_plugin_free(void* ptr);
}
```

```cpp
struct PluginInfo {
    const char* name;           // "ffmpeg"
    ETLType etl;                // ETL_TRANSFORM
    const char** targets;       // ["mp4", "webm", "gif", ...]
    const char* version;
};

struct Request {
    const char* input_path;     // 입력 파일 경로
    const char* output_path;    // 출력 파일 경로
    const char* target;         // 타겟 (예: "jpg")
    const char* options_json;   // 플러그인 옵션 (JSON)
};

struct Result {
    bool success;
    const char* output_path;    // 실제 출력 경로
    const char* error;          // 에러 메시지 (실패 시)
    const char* metadata_json;  // 추가 메타데이터 (JSON)
};
```

#### CLI 플러그인 (언어 무관)

개발 편의성 우선:

```json
{
  "name": "face-extractor",
  "etl": "extract",
  "targets": ["faces"],
  "executable": "face-extractor",
  "interface": "cli"
}
```

CLI 플러그인 호출 규약:

```bash
<executable> --input <path> --output <path> --target <target> [--input-format <fmt>] [플러그인 옵션]
```

**참고:** 다단계 파이프라인에서 중간 파일은 `.tmp` 확장자를 사용. 이때 `--input-format`으로 실제 포맷 힌트가 전달됨.

stdout으로 JSON 결과 출력:

```json
{
  "success": true,
  "output_path": "/path/to/output",
  "metadata": { ... }
}
```

아무 언어로 개발 가능:

```python
#!/usr/bin/env python3
# face-extractor (Python)
import sys, json, argparse

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--input', required=True)
    parser.add_argument('--output', required=True)
    parser.add_argument('--target', required=True)
    parser.add_argument('--confidence', type=float, default=0.5)
    args = parser.parse_args()

    # 처리 로직
    results = extract_faces(args.input, args.confidence)
    save_results(results, args.output)

    # JSON 결과 stdout 출력
    print(json.dumps({
        "success": True,
        "output_path": args.output,
        "metadata": {"count": len(results)}
    }))

if __name__ == '__main__':
    main()
```

### 4.4 플러그인 매니페스트

```json
{
  "name": "ai-vision",
  "version": "1.2.0",
  "etl": "extract",
  "targets": ["faces", "text", "objects", "labels"],
  "interface": "cli",
  "executable": "ai-vision-extract",
  "options": [
    { "name": "--confidence", "type": "float", "default": 0.5 },
    { "name": "--language", "type": "string", "default": "auto" }
  ]
}
```

### 4.5 플러그인 검색 경로

```
~/.uniconv/plugins/          # 유저 설치
/usr/local/lib/uniconv/      # 시스템 설치
./plugins/                   # 로컬 (개발용)
```

### 4.6 플러그인 설치 및 관리

```bash
# 개별 설치
uniconv plugin install ffmpeg.transform
uniconv plugin install ffmpeg.extract

# 스코프 전체 설치
uniconv plugin install ffmpeg

# 컬렉션 설치
uniconv plugin install +essentials

# 온라인에서 검색/설치
uniconv plugin search face
uniconv plugin install ai-vision

# 플러그인 제거
uniconv plugin remove ai-vision

# 플러그인 업데이트
uniconv plugin update ai-vision
uniconv plugin update --all

# 목록 조회
uniconv plugin list

# 특정 타겟 지원 플러그인 조회
uniconv plugin list --target faces

# 스코프 상세 조회
uniconv plugin list --scope ffmpeg

# 기본 플러그인 설정
uniconv config set default.jpg vips
uniconv config set default.faces mediapipe
```

### 4.7 플러그인 조회 예시

```bash
$ uniconv plugin list

NAME                     TARGETS                       VERSION   SOURCE
-----------------------------------------------------------------------------
ascii                    ascii,ascii-art,text-art      1.0.7     registry
image-filter             grayscale,gray,bw,invert...   1.0.5     registry
image-convert            jpg,jpeg,png,webp,gif,...     1.0.16    registry
doc-convert              pdf,docx,doc,odt,rtf,...      1.0.6     registry
video-convert            mp4,mov,avi,webm,mkv,...      1.1.11    registry

$ uniconv plugin info image-convert

Name:        image-convert
Scope:       uniconv
Version:     1.0.16
Description: Image format conversion via libvips
Type:        native
Source:      registry
Targets:     jpg, jpeg, png, webp, gif, bmp, tiff, heic, avif
Inputs:      jpg, jpeg, png, webp, gif, bmp, tiff, heic, avif, pdf
```

### 4.8 플러그인 충돌 해결

같은 타겟을 여러 플러그인이 지원하는 경우:

```bash
# 기본값 미설정 + interactive
uniconv "photo.jpg | faces"
# → "어떤 플러그인을 사용할까요?"
# → [1] ai-vision (recommended)
# → [2] face-yolo
# → [3] face-mediapipe

# 기본값 미설정 + non-interactive
uniconv --no-interactive "photo.jpg | faces"
# → 경고 출력 후 첫 번째 사용
# ⚠ Multiple plugins support 'faces'. Using 'ai-vision'.
#   Use faces@mediapipe to specify.

# 명시적 지정
uniconv "photo.jpg | faces@mediapipe"

# 기본값 설정
uniconv config set default.faces mediapipe
```

---

## 5. 플러그인 예시

### 5.1 Transform 플러그인

- **pdf-tools.transform**: PDF 병합, 분할, 압축
- **hwp.transform**: HWP ↔ PDF/DOCX
- **office.transform**: DOCX, XLSX, PPTX 변환
- **cad.transform**: DWG, DXF 변환
- **3d.transform**: STL, OBJ, FBX, GLTF 변환
- **raw.transform**: RAW (CR2, NEF, ARW) 변환
- **ai-image.transform**: 배경 제거, 업스케일, 보정

### 5.2 Extract 플러그인

**이미지 분석:**

- **ai-vision.extract**: faces, text, objects, labels
- **ocr.extract**: text, tables, forms
- **document.extract**: receipt, invoice, business-card, resume

**비디오 분석:**

- **video-ai.extract**: scenes, highlights, summary
- **speech.extract**: transcript, chapters
- **face-tracker.extract**: person (특정 인물 추적)

**검색/분류:**

- **semantic.extract**: 의미 검색 ("beach sunset")
- **similarity.extract**: similar (유사 이미지)
- **dedup.extract**: duplicates (중복 찾기)

**오디오 분석:**

- **audio-ai.extract**: stems (보컬/악기 분리)
- **transcribe.extract**: transcript, minutes

### 5.3 Load 플러그인

- **gdrive.load**: Google Drive
- **s3.load**: AWS S3
- **dropbox.load**: Dropbox
- **notion.load**: Notion
- **slack.load**: Slack

---

## 6. CLI 상세

### 6.1 Core 옵션

Core 옵션은 소스 파일 앞에 위치:

```bash
uniconv [core옵션] <source> "<pipeline>"
```

| 옵션                  | 설명                  |
| --------------------- | --------------------- |
| `-o, --output <path>` | 출력 경로             |
| `-f, --force`         | 덮어쓰기              |
| `--json`              | JSON 출력             |
| `--quiet`             | 조용히                |
| `--verbose`           | 상세 로그             |
| `--dry-run`           | 실제 실행 안 함       |
| `-r, --recursive`     | 재귀적 디렉토리 처리  |
| `--no-interactive`    | interactive 모드 끄기 |
| `--interactive`       | interactive 모드 강제 |
| `--preset <n>`        | 프리셋 사용           |

**플러그인별 옵션** (파이프라인 내 타겟 뒤에 위치):
- `--quality`, `--width`, `--height` 등은 각 타겟의 플러그인별 옵션
- 예: `uniconv "photo.heic | jpg --quality 90 --width 1920"`

**출력 경로 결정 우선순위**:
1. 파이프라인 요소 내 `--output` 옵션 (예: `jpg --output out.jpg`)
2. CLI `-o` 옵션 (마지막 스테이지에만 적용):
   - 확장자 없음: 타겟 확장자 추가 (`-o ./temp/output` → `./temp/output.png`)
   - 확장자 있음: 그대로 사용 (`-o ./temp/output.png` → `./temp/output.png`)
   - 참고: tee로 복수 출력 시 확장자가 지정되면 같은 파일에 덮어쓰기됨
3. 기본값: 현재 디렉토리에 입력 파일명 + 타겟 확장자

### 6.2 조회 명령어

```bash
uniconv info <file>                  # 파일 상세 정보
uniconv formats                      # 지원 포맷 목록
uniconv preset list                  # 프리셋 목록
uniconv plugin list                  # 플러그인 목록
uniconv plugin list --target <t>     # 특정 타겟 지원 플러그인
uniconv plugin list --scope <s>      # 특정 스코프 플러그인
```

### 6.3 관리 명령어

```bash
# 프리셋 관리
uniconv preset create <n> "<pipeline>"
uniconv preset delete <n>
uniconv preset show <n>
uniconv preset list

# 플러그인 관리
uniconv plugin install <n>
uniconv plugin remove <n>
uniconv plugin update [name | --all]
uniconv plugin search <keyword>
uniconv plugin info <n>

# 설정
uniconv config get <key>
uniconv config set <key> <value>
uniconv config list
```

### 6.4 JSON 출력

```bash
$ uniconv --json photo.heic "jpg | grayscale"
```

```json
{
  "success": true,
  "stage_results": [
    {
      "stage_index": 0,
      "target": "jpg",
      "plugin_used": "image-convert",
      "input": "photo.heic",
      "output": "/tmp/uniconv/stage0_elem0_jpg_photo.tmp",
      "status": "success"
    },
    {
      "stage_index": 1,
      "target": "grayscale",
      "plugin_used": "image-filter",
      "input": "/tmp/uniconv/stage0_elem0_jpg_photo.tmp",
      "output": "/path/to/photo_grayscale.jpg",
      "status": "success"
    }
  ],
  "final_outputs": ["/path/to/photo_grayscale.jpg"]
}
```

---

## 7. 프리셋 시스템

### 7.1 프리셋 생성

프리셋 = 저장된 파이프라인

```bash
# 파이프라인을 프리셋으로 저장
uniconv preset create insta "jpg --quality 85 --width 1080"
uniconv preset create multi-format "tee | jpg, png, webp"
uniconv preset create video-gif "gif --width 320 --fps 15"
```

### 7.2 프리셋 사용

```bash
uniconv --preset insta photo.heic
# = uniconv photo.heic "jpg --quality 85 --width 1080"

uniconv --preset multi-format photo.jpg
# = uniconv photo.jpg "tee | jpg, png, webp"

uniconv --preset video-gif video.mp4
# = uniconv video.mp4 "gif --width 320 --fps 15"
```

### 7.3 프리셋 저장 위치

```
~/.uniconv/presets/
├── insta.json
├── backup.json
└── video-gif.json
```

### 7.4 프리셋 포맷

```json
{
  "name": "insta",
  "description": "Instagram optimized",
  "pipeline": "jpg --quality 85 --width 1080"
}
```

---

## 8. Watch 모드

```bash
# 기본: 폴더 감시하며 자동 변환
uniconv watch ./incoming "jpg"

# 프리셋과 함께
uniconv watch --preset insta ./incoming

# 출력 경로 지정
uniconv watch -o ./processed ./incoming "jpg"

# 파이프라인
uniconv watch ./incoming "jpg | grayscale"

# 재귀적 감시
uniconv watch -r ./incoming "jpg"
```

---

## 9. 플랫폼별 컨텍스트 메뉴

### 9.1 macOS

- Quick Action (Automator workflow) 설치
- `~/Library/Services/` 에 workflow 복사

```bash
uniconv --install-context-menu
```

### 9.2 Windows

- 레지스트리 등록
- `HKEY_CLASSES_ROOT\*\shell\uniconv`

```powershell
uniconv --install-context-menu
```

### 9.3 Linux

- Nautilus scripts (GNOME)
- Dolphin service menus (KDE)

```bash
uniconv --install-context-menu
```

### 9.4 동작 방식

컨텍스트 메뉴 클릭 시:

1. uniconv CLI interactive 모드 실행, 또는
2. uniconv GUI (별도 앱) 실행

---

## 10. 기술 스택

### 10.1 Core

- **언어**: C++20
- **빌드**: CMake
- **CLI 파싱**: CLI11
- **JSON**: nlohmann/json
- **비디오/오디오**: FFmpeg (libav\*)
- **이미지**: libvips (외부 플러그인 `image-convert` 통해 제공)

### 10.2 플러그인

- **Native**: C ABI (.so, .dylib, .dll)
- **CLI 기반**: 언어 무관 (Python, Go, Rust, Node.js, ...)

### 10.3 크로스 플랫폼 빌드

- **CI**: GitHub Actions
- **macOS**: Clang + Homebrew
- **Windows**: MSVC + vcpkg
- **Linux**: GCC + apt

---

## 11. 코딩 컨벤션

### 11.1 네이밍 규칙

| 종류          | 스타일           | 예시                  |
| ------------- | ---------------- | --------------------- |
| 파일명        | snake_case       | `plugin_manager.cpp`  |
| 헤더 (C++)    | .hpp             | `plugin_manager.hpp`  |
| 헤더 (C ABI)  | .h               | `plugin.h`            |
| 클래스/구조체 | PascalCase       | `PluginManager`       |
| 함수          | snake_case       | `load_plugin()`       |
| 변수          | snake_case       | `file_path`           |
| 멤버 변수     | snake*case + `*` | `plugins_`, `config_` |
| 상수          | kPascalCase      | `kDefaultQuality`     |
| 매크로        | UPPER_SNAKE      | `UNICONV_VERSION`     |
| 네임스페이스  | snake_case       | `uniconv::core`       |
| 인터페이스    | I + PascalCase   | `IPlugin`             |
| 열거형        | PascalCase       | `ETLType::Transform`  |

### 11.2 코드 예시

```cpp
namespace uniconv::core {

// ETL 타입
enum class ETLType {
    Transform,
    Extract,
    Load
};

// 플러그인 정보
struct PluginInfo {
    std::string name;
    ETLType etl;
    std::vector<std::string> targets;
    std::string version;
    bool builtin;
};

// 파이프라인 스테이지
struct Stage {
    std::string target;
    std::string plugin;           // 명시적 지정 시
    std::map<std::string, std::string> options;
};

// 파이프라인
struct Pipeline {
    std::string source;
    std::vector<std::vector<Stage>> stages;  // 각 스테이지는 병렬 요소 가능
};

// 플러그인 인터페이스
class IPlugin {
public:
    virtual ~IPlugin() = default;
    virtual PluginInfo info() const = 0;
    virtual Result execute(const Request& req) = 0;
};

} // namespace uniconv::core
```

---

## 12. 디렉토리 구조

```
uniconv/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── cli/
│   │   ├── parser.cpp
│   │   ├── pipeline_parser.cpp    # 파이프라인 문법 파싱
│   │   ├── interactive.cpp
│   │   └── commands/
│   │       ├── run.cpp            # 파이프라인 실행
│   │       ├── info.cpp
│   │       ├── preset.cpp
│   │       └── plugin.cpp
│   ├── core/
│   │   ├── engine.cpp
│   │   ├── pipeline_executor.cpp  # 파이프라인 실행 엔진
│   │   ├── plugin_manager.cpp
│   │   ├── plugin_loader_native.cpp
│   │   ├── plugin_loader_cli.cpp
│   │   ├── preset_manager.cpp
│   │   └── watcher.cpp
│   ├── builtins/
│   │   └── tee.cpp                # tee builtin
│   ├── plugins/                   # 플러그인 인터페이스
│   │   ├── plugin_interface.h     # IPlugin base class
│   │   ├── ffmpeg_transform.cpp
│   │   └── ffmpeg_extract.cpp
│   └── utils/
│       ├── file_utils.cpp
│       └── json_output.cpp
├── include/
│   └── uniconv/
│       ├── plugin.h               # 플러그인 C ABI
│       ├── pipeline.hpp
│       ├── types.hpp
│       └── ...
├── plugins/
│   └── examples/                  # 예제 플러그인
│       ├── python/
│       ├── go/
│       └── rust/
├── scripts/
│   ├── macos/
│   ├── windows/
│   └── linux/
└── tests/
```

---

## 13. 향후 확장

### 13.1 GUI 앱

- 컨텍스트 메뉴에서 호출되는 옵션 선택 UI
- 플랫폼별 네이티브 (SwiftUI, WinUI, GTK)
- CLI core 호출
- 파이프라인 시각적 편집기

### 13.2 플러그인 레지스트리

- 중앙 플러그인 저장소
- `uniconv plugin search <keyword>`
- 버전 관리, 의존성 해결

### 13.3 Python/Node SDK

- 플러그인 개발용 SDK
- 보일러플레이트 생성기

```bash
uniconv plugin init --lang python my-plugin
```

---

## 14. 마일스톤

### Phase 1: MVP ✅

- [x] 파이프라인 파서
- [x] 기본 파이프라인 실행 (단일 스테이지)
- [x] 이미지 변환 (HEIC, JPG, PNG, WebP)
- [x] JSON 출력

### Phase 2: 파이프라인 확장 ✅

- [x] 다중 스테이지 파이프라인
- [x] tee builtin (분기)
- [x] 프리셋 시스템
- [x] clipboard builtin
- [x] passthrough builtin (_)
- [x] `--input-format` 힌트 (다단계 파이프라인용)

### Phase 3: 플러그인 시스템 ✅

- [x] Native 플러그인 로더
- [x] CLI 플러그인 로더
- [x] 플러그인 관리 명령어
- [x] 플러그인 레지스트리
- [x] 플러그인 컬렉션 (+essentials)
- [x] Python 의존성 자동 설치

### Phase 4: 확장 기능 (진행 중)

- [x] 비디오 변환 (video-convert 플러그인)
- [x] 문서 변환 (doc-convert 플러그인)
- [x] Watch 모드
- [ ] Interactive 모드

### Phase 5: 플랫폼 통합

- [ ] macOS 컨텍스트 메뉴
- [ ] Windows 컨텍스트 메뉴
- [ ] Linux 컨텍스트 메뉴

### Phase 6: AI 및 생태계

- [ ] Extract 플러그인 (faces, text, ...)
- [ ] Load 플러그인 (gdrive, s3, ...)
- [ ] 플러그인 SDK
