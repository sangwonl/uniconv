#include "preset_command.h"
#include <sstream>

namespace uniconv::cli::commands
{

    PresetCommand::PresetCommand(std::shared_ptr<core::PresetManager> preset_manager,
                                 std::shared_ptr<core::output::IOutput> output)
        : preset_manager_(std::move(preset_manager)), output_(std::move(output))
    {
    }

    int PresetCommand::execute(const ParsedArgs &args)
    {
        if (args.subcommand_args.empty())
        {
            output_->error("No subcommand specified");
            return 1;
        }

        const auto &subcmd = args.subcommand_args[0];

        if (subcmd == "list")
            return list(args);
        if (subcmd == "create")
            return create_preset(args);
        if (subcmd == "delete")
            return delete_preset(args);
        if (subcmd == "show")
            return show_preset(args);
        if (subcmd == "export")
            return export_preset(args);
        if (subcmd == "import")
            return import_preset(args);

        output_->error("Unknown preset subcommand: " + subcmd);
        return 1;
    }

    int PresetCommand::list(const ParsedArgs & /*args*/)
    {
        auto presets = preset_manager_->list();

        nlohmann::json j = nlohmann::json::array();
        for (const auto &p : presets)
        {
            j.push_back(p.to_json());
        }

        std::ostringstream text;
        if (presets.empty())
        {
            text << "No presets found.\n";
            text << "Create one with: uniconv preset create <name> -t <format> [options]";
        }
        else
        {
            text << "Available presets:";
            for (const auto &p : presets)
            {
                text << "\n  " << p.name;
                if (!p.description.empty())
                {
                    text << " - " << p.description;
                }
                text << " (-> " << p.target << ")";
            }
        }

        output_->data(j, text.str());
        return 0;
    }

    int PresetCommand::create_preset(const ParsedArgs &args)
    {
        if (args.subcommand.empty())
        {
            output_->error("Preset name required");
            return 1;
        }

        // TODO: Implement preset creation from pipeline string
        // For now, presets should be created by specifying a pipeline string
        // Example: uniconv preset create instagram "jpg --quality 85 --width 1080"
        output_->error("Preset creation not yet implemented for pipeline syntax");
        output_->info("Usage: uniconv preset create <name> \"<pipeline>\"");
        return 1;
    }

    int PresetCommand::delete_preset(const ParsedArgs &args)
    {
        if (args.subcommand.empty())
        {
            output_->error("Preset name required");
            return 1;
        }

        if (preset_manager_->remove(args.subcommand))
        {
            nlohmann::json j;
            j["success"] = true;
            j["deleted"] = args.subcommand;
            output_->data(j, "Deleted preset: " + args.subcommand);
            return 0;
        }
        else
        {
            output_->error("Preset not found: " + args.subcommand);
            return 1;
        }
    }

    int PresetCommand::show_preset(const ParsedArgs &args)
    {
        if (args.subcommand.empty())
        {
            output_->error("Preset name required");
            return 1;
        }

        auto preset = preset_manager_->load(args.subcommand);
        if (!preset)
        {
            output_->error("Preset not found: " + args.subcommand);
            return 1;
        }

        std::ostringstream text;
        text << "Name: " << preset->name;
        if (!preset->description.empty())
        {
            text << "\nDescription: " << preset->description;
        }
        text << "\nTarget: " << preset->target;
        if (preset->plugin)
        {
            text << "\nPlugin: " << *preset->plugin;
        }
        if (!preset->plugin_options.empty())
        {
            text << "\nOptions:";
            for (const auto &opt : preset->plugin_options)
            {
                text << " " << opt;
            }
        }

        output_->data(preset->to_json(), text.str());
        return 0;
    }

    int PresetCommand::export_preset(const ParsedArgs &args)
    {
        if (args.subcommand.empty())
        {
            output_->error("Preset name required");
            return 1;
        }

        if (args.subcommand_args.size() < 2)
        {
            output_->error("Output file required");
            return 1;
        }

        try
        {
            preset_manager_->export_preset(args.subcommand, args.subcommand_args[1]);
            nlohmann::json j;
            j["success"] = true;
            j["preset"] = args.subcommand;
            j["file"] = args.subcommand_args[1];
            output_->data(j, "Exported preset to: " + args.subcommand_args[1]);
            return 0;
        }
        catch (const std::exception &e)
        {
            output_->error(e.what());
            return 1;
        }
    }

    int PresetCommand::import_preset(const ParsedArgs &args)
    {
        if (args.subcommand_args.size() < 2)
        {
            output_->error("Input file required");
            return 1;
        }

        try
        {
            preset_manager_->import_preset(args.subcommand_args[1]);
            nlohmann::json j;
            j["success"] = true;
            j["file"] = args.subcommand_args[1];
            output_->data(j, "Imported preset from: " + args.subcommand_args[1]);
            return 0;
        }
        catch (const std::exception &e)
        {
            output_->error(e.what());
            return 1;
        }
    }

} // namespace uniconv::cli::commands
