#include <gtest/gtest.h>
#include <fstream>
#include "cli/pipeline_parser.h"
#include "builtins/collect.h"
#include "core/execution_graph.h"

using namespace uniconv;

// ============================================================================
// Pipeline Parser: Collect Recognition
// ============================================================================

class ScatterCollectParserTest : public ::testing::Test
{
protected:
    cli::PipelineParser parser;
    std::filesystem::path test_source = "/path/to/input.zip";
    core::CoreOptions default_options;
};

TEST_F(ScatterCollectParserTest, CollectAsStage)
{
    // "decompress | png | collect | zip" — collect appears as a normal stage
    auto result = parser.parse("decompress | png | collect | zip", test_source, default_options);

    ASSERT_TRUE(result.success) << "Parse failed: " << result.error;
    ASSERT_EQ(result.pipeline.stages.size(), 4);

    EXPECT_EQ(result.pipeline.stages[0].elements[0].target, "decompress");
    EXPECT_EQ(result.pipeline.stages[1].elements[0].target, "png");
    EXPECT_EQ(result.pipeline.stages[2].elements[0].target, "collect");
    EXPECT_TRUE(result.pipeline.stages[2].elements[0].is_collect());
    EXPECT_TRUE(result.pipeline.stages[2].has_collect());
    EXPECT_EQ(result.pipeline.stages[3].elements[0].target, "zip");
}

TEST_F(ScatterCollectParserTest, CollectAfterTee)
{
    // "tee | jpg, png | collect | zip"
    // This uses a static N (2) from tee, then collect gathers them
    auto result = parser.parse("tee | jpg, png | collect | zip", test_source, default_options);

    ASSERT_TRUE(result.success) << "Parse failed: " << result.error;
    ASSERT_EQ(result.pipeline.stages.size(), 4);

    EXPECT_TRUE(result.pipeline.stages[0].has_tee());
    ASSERT_EQ(result.pipeline.stages[1].elements.size(), 2);
    EXPECT_TRUE(result.pipeline.stages[2].has_collect());
    EXPECT_EQ(result.pipeline.stages[3].elements[0].target, "zip");
}

TEST_F(ScatterCollectParserTest, CollectSimpleGather)
{
    // "tee | jpg, png | collect"
    // Collect as terminal stage
    auto result = parser.parse("tee | jpg, png | collect", test_source, default_options);

    ASSERT_TRUE(result.success) << "Parse failed: " << result.error;
    ASSERT_EQ(result.pipeline.stages.size(), 3);
    EXPECT_TRUE(result.pipeline.stages[2].has_collect());
}

// ============================================================================
// Pipeline Validation: Collect Rules
// ============================================================================

TEST_F(ScatterCollectParserTest, CollectCannotBeFirstStage)
{
    auto result = parser.parse("collect", test_source, default_options);

    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.error.find("collect") != std::string::npos ||
                result.error.find("first") != std::string::npos);
}

TEST_F(ScatterCollectParserTest, CollectWithMultipleElementsFails)
{
    // collect must be the only element in its stage
    // This is tricky to construct; need a pipeline where stage has collect + something
    // "tee | collect, jpg" - this creates a stage with 2 elements including collect
    // But first we need preceding width > 1, which tee provides
    // Actually, "tee | collect, jpg" would be width 1 (tee) → width 2 next stage,
    // which is valid for tee. But collect must be alone in its stage.
    auto result = parser.parse("tee | jpg, png | collect, zip", test_source, default_options);

    // Should fail because collect is not the only element in stage 2
    EXPECT_FALSE(result.success);
}

TEST_F(ScatterCollectParserTest, NtoCollectValid)
{
    // N → 1 (collect) should be valid
    auto result = parser.parse("tee | jpg, png | collect", test_source, default_options);

    ASSERT_TRUE(result.success) << "Parse failed: " << result.error;
}

// ============================================================================
// StageElement: is_collect() detection
// ============================================================================

TEST(StageElementTest, IsCollect)
{
    core::StageElement elem;
    elem.target = "collect";
    EXPECT_TRUE(elem.is_collect());
    EXPECT_FALSE(elem.is_tee());
    EXPECT_FALSE(elem.is_clipboard());
    EXPECT_FALSE(elem.is_passthrough());
}

TEST(StageElementTest, IsNotCollect)
{
    core::StageElement elem;
    elem.target = "jpg";
    EXPECT_FALSE(elem.is_collect());
}

// ============================================================================
// Collect Builtin
// ============================================================================

class CollectBuiltinTest : public ::testing::Test
{
protected:
    std::filesystem::path temp_dir;

    void SetUp() override
    {
        temp_dir = std::filesystem::temp_directory_path() / "uniconv_test_collect";
        std::filesystem::create_directories(temp_dir);
    }

    void TearDown() override
    {
        std::filesystem::remove_all(temp_dir);
    }

    // Helper to create a temp file with content
    std::filesystem::path create_temp_file(const std::string &name, const std::string &content)
    {
        auto path = temp_dir / name;
        std::ofstream f(path);
        f << content;
        f.close();
        return path;
    }
};

TEST_F(CollectBuiltinTest, CollectMultipleFiles)
{
    auto file1 = create_temp_file("a.txt", "hello");
    auto file2 = create_temp_file("b.txt", "world");
    auto file3 = create_temp_file("c.txt", "test");

    std::vector<std::filesystem::path> inputs = {file1, file2, file3};
    auto result = builtins::Collect::execute(inputs, temp_dir);

    ASSERT_TRUE(result.success) << "Collect failed: " << result.error;
    EXPECT_TRUE(std::filesystem::is_directory(result.output_dir));

    // Check files exist in output directory with ordered names
    size_t file_count = 0;
    for (const auto &entry : std::filesystem::directory_iterator(result.output_dir))
    {
        file_count++;
        // Files should be named like 0000_a.txt, 0001_b.txt, 0002_c.txt
        (void)entry;
    }
    EXPECT_EQ(file_count, 3);
}

TEST_F(CollectBuiltinTest, CollectSingleFile)
{
    auto file1 = create_temp_file("single.txt", "content");

    std::vector<std::filesystem::path> inputs = {file1};
    auto result = builtins::Collect::execute(inputs, temp_dir);

    ASSERT_TRUE(result.success) << "Collect failed: " << result.error;
    EXPECT_TRUE(std::filesystem::is_directory(result.output_dir));
}

TEST_F(CollectBuiltinTest, CollectEmptyInputsFails)
{
    std::vector<std::filesystem::path> inputs;
    auto result = builtins::Collect::execute(inputs, temp_dir);

    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error.empty());
}

TEST_F(CollectBuiltinTest, CollectMissingFileFails)
{
    std::vector<std::filesystem::path> inputs = {temp_dir / "nonexistent.txt"};
    auto result = builtins::Collect::execute(inputs, temp_dir);

    EXPECT_FALSE(result.success);
}

TEST_F(CollectBuiltinTest, CollectPreservesOrder)
{
    // Create files in reverse alphabetical order to test ordering
    auto file_c = create_temp_file("c.txt", "third");
    auto file_a = create_temp_file("a.txt", "first");
    auto file_b = create_temp_file("b.txt", "second");

    // Pass in specific order: a, b, c
    std::vector<std::filesystem::path> inputs = {file_a, file_b, file_c};
    auto result = builtins::Collect::execute(inputs, temp_dir);

    ASSERT_TRUE(result.success);

    // Collect ordered files and verify order is preserved
    std::vector<std::string> collected_names;
    for (const auto &entry : std::filesystem::directory_iterator(result.output_dir))
    {
        collected_names.push_back(entry.path().filename().string());
    }
    std::sort(collected_names.begin(), collected_names.end());

    ASSERT_EQ(collected_names.size(), 3);
    EXPECT_EQ(collected_names[0], "0000_a.txt");
    EXPECT_EQ(collected_names[1], "0001_b.txt");
    EXPECT_EQ(collected_names[2], "0002_c.txt");
}

// ============================================================================
// Collect Builtin: Validation
// ============================================================================

TEST(CollectValidationTest, CannotBeFirstStage)
{
    auto result = builtins::Collect::validate(0, 3);
    EXPECT_FALSE(result.valid);
}

TEST(CollectValidationTest, ValidMiddleStage)
{
    auto result = builtins::Collect::validate(1, 3);
    EXPECT_TRUE(result.valid);
}

TEST(CollectValidationTest, ValidLastStage)
{
    auto result = builtins::Collect::validate(2, 3);
    EXPECT_TRUE(result.valid);
}

// ============================================================================
// Collect::is_collect
// ============================================================================

TEST(CollectIsCollectTest, MatchesCollect)
{
    EXPECT_TRUE(builtins::Collect::is_collect("collect"));
    EXPECT_TRUE(builtins::Collect::is_collect("Collect"));
    EXPECT_TRUE(builtins::Collect::is_collect("COLLECT"));
}

TEST(CollectIsCollectTest, DoesNotMatchOther)
{
    EXPECT_FALSE(builtins::Collect::is_collect("tee"));
    EXPECT_FALSE(builtins::Collect::is_collect("jpg"));
    EXPECT_FALSE(builtins::Collect::is_collect("collector"));
}

// ============================================================================
// ExecutionGraph: Collect Node Building
// ============================================================================

TEST(ExecutionGraphCollectTest, CollectNodeCreated)
{
    core::Pipeline pipeline;
    pipeline.source = "/test/input.zip";

    // Stage 0: tee
    core::PipelineStage tee_stage;
    core::StageElement tee_elem;
    tee_elem.target = "tee";
    tee_stage.elements.push_back(tee_elem);
    pipeline.stages.push_back(tee_stage);

    // Stage 1: jpg, png (parallel elements)
    core::PipelineStage convert_stage;
    core::StageElement jpg_elem;
    jpg_elem.target = "jpg";
    convert_stage.elements.push_back(jpg_elem);
    core::StageElement png_elem;
    png_elem.target = "png";
    convert_stage.elements.push_back(png_elem);
    pipeline.stages.push_back(convert_stage);

    // Stage 2: collect
    core::PipelineStage collect_stage;
    core::StageElement collect_elem;
    collect_elem.target = "collect";
    collect_stage.elements.push_back(collect_elem);
    pipeline.stages.push_back(collect_stage);

    core::ExecutionGraph graph;
    graph.build_from_pipeline(pipeline);

    // Find the collect node
    bool found_collect = false;
    for (const auto &node : graph.nodes())
    {
        if (node.is_collect)
        {
            found_collect = true;
            EXPECT_EQ(node.target, "collect");
            EXPECT_EQ(node.stage_idx, 2);
            // Collect should have 2 input nodes (jpg and png)
            EXPECT_EQ(node.input_nodes.size(), 2);
        }
    }
    EXPECT_TRUE(found_collect);
}

// ============================================================================
// Result: Scatter Detection
// ============================================================================

TEST(ResultScatterTest, SingleOutputIsNotScatter)
{
    core::Result r;
    r.output = "/path/to/file.png";
    EXPECT_FALSE(r.is_scatter());
}

TEST(ResultScatterTest, MultipleOutputsIsScatter)
{
    core::Result r;
    r.outputs = {"/path/frame_001.png", "/path/frame_002.png", "/path/frame_003.png"};
    EXPECT_TRUE(r.is_scatter());
}

TEST(ResultScatterTest, EmptyOutputsIsNotScatter)
{
    core::Result r;
    EXPECT_FALSE(r.is_scatter());
}
