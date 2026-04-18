#include <cstddef>
#include <cstdio>
#include <string>
#include <pipewire/context.h>
#include <pipewire/core.h>
#include <pipewire/filter.h>
#include <pipewire/impl-module.h>
#include <pipewire/keys.h>
#include <pipewire/main-loop.h>
#include <pipewire/pipewire.h>
#include <pipewire/properties.h>

using std::string;

int main(int argc, char *argv[]) {
    // Initialize PipeWire library
    pw_init(&argc, &argv);

    // Debug: Output current version of PipeWire library
    fprintf(stdout,
            "Compiled with libpipewire %s\n"
            "Linked with libpipewire %s\n",
            pw_get_headers_version(), pw_get_library_version());

    // Create some initial stuff for PipeWire to work
    struct pw_main_loop *loop = pw_main_loop_new(NULL);
    if (!loop) {
        fprintf(stderr, "Failed to create PipeWire main loop\n");
        return -1;
    }
    struct pw_context *context = pw_context_new(pw_main_loop_get_loop(loop), NULL, 0);
    if (!context) {
        fprintf(stderr, "Failed to create PipeWire context\n");
        return -1;
    }
    /*struct pw_core *core = pw_context_connect(context, NULL, 0);
    if (!core) {
        fprintf(stderr, "Failed to connect to PipeWire daemon\n");
        return -1;
    }*/

    // Set filter graph for virtual surround module
    // TODO: Use variable for filename strings
    const char *filter_graph = R"(
{
    nodes = [
        # duplicate inputs
        { type = builtin label = copy name = copyFL  }
        { type = builtin label = copy name = copyFR  }
        { type = builtin label = copy name = copyFC  }
        { type = builtin label = copy name = copyRL  }
        { type = builtin label = copy name = copyRR  }
        { type = builtin label = copy name = copySL  }
        { type = builtin label = copy name = copySR  }
        { type = builtin label = copy name = copyLFE }

        # apply hrir - HeSuVi 14-channel WAV (not the *-.wav variants) (note: */44/* in HeSuVi are the same, but resampled to 44100)
        { type = builtin label = convolver name = convFL_L config = { filename = "/home/berny23/Dokumente/Virtual Surround Sound/hrir_hesuvi/atmos.wav" channel =  0 } }
        { type = builtin label = convolver name = convFL_R config = { filename = "/home/berny23/Dokumente/Virtual Surround Sound/hrir_hesuvi/atmos.wav" channel =  1 } }
        { type = builtin label = convolver name = convSL_L config = { filename = "/home/berny23/Dokumente/Virtual Surround Sound/hrir_hesuvi/atmos.wav" channel =  2 } }
        { type = builtin label = convolver name = convSL_R config = { filename = "/home/berny23/Dokumente/Virtual Surround Sound/hrir_hesuvi/atmos.wav" channel =  3 } }
        { type = builtin label = convolver name = convRL_L config = { filename = "/home/berny23/Dokumente/Virtual Surround Sound/hrir_hesuvi/atmos.wav" channel =  4 } }
        { type = builtin label = convolver name = convRL_R config = { filename = "/home/berny23/Dokumente/Virtual Surround Sound/hrir_hesuvi/atmos.wav" channel =  5 } }
        { type = builtin label = convolver name = convFC_L config = { filename = "/home/berny23/Dokumente/Virtual Surround Sound/hrir_hesuvi/atmos.wav" channel =  6 } }
        { type = builtin label = convolver name = convFR_R config = { filename = "/home/berny23/Dokumente/Virtual Surround Sound/hrir_hesuvi/atmos.wav" channel =  7 } }
        { type = builtin label = convolver name = convFR_L config = { filename = "/home/berny23/Dokumente/Virtual Surround Sound/hrir_hesuvi/atmos.wav" channel =  8 } }
        { type = builtin label = convolver name = convSR_R config = { filename = "/home/berny23/Dokumente/Virtual Surround Sound/hrir_hesuvi/atmos.wav" channel =  9 } }
        { type = builtin label = convolver name = convSR_L config = { filename = "/home/berny23/Dokumente/Virtual Surround Sound/hrir_hesuvi/atmos.wav" channel = 10 } }
        { type = builtin label = convolver name = convRR_R config = { filename = "/home/berny23/Dokumente/Virtual Surround Sound/hrir_hesuvi/atmos.wav" channel = 11 } }
        { type = builtin label = convolver name = convRR_L config = { filename = "/home/berny23/Dokumente/Virtual Surround Sound/hrir_hesuvi/atmos.wav" channel = 12 } }
        { type = builtin label = convolver name = convFC_R config = { filename = "/home/berny23/Dokumente/Virtual Surround Sound/hrir_hesuvi/atmos.wav" channel = 13 } }

        # treat LFE as FC
        { type = builtin label = convolver name = convLFE_L config = { filename = "/home/berny23/Dokumente/Virtual Surround Sound/hrir_hesuvi/atmos.wav" channel =  6 } }
        { type = builtin label = convolver name = convLFE_R config = { filename = "/home/berny23/Dokumente/Virtual Surround Sound/hrir_hesuvi/atmos.wav" channel = 13 } }

        # stereo output
        { type = builtin label = mixer name = mixL }
        { type = builtin label = mixer name = mixR }
    ]
    links = [
        # input
        { output = "copyFL:Out"  input="convFL_L:In"  }
        { output = "copyFL:Out"  input="convFL_R:In"  }
        { output = "copySL:Out"  input="convSL_L:In"  }
        { output = "copySL:Out"  input="convSL_R:In"  }
        { output = "copyRL:Out"  input="convRL_L:In"  }
        { output = "copyRL:Out"  input="convRL_R:In"  }
        { output = "copyFC:Out"  input="convFC_L:In"  }
        { output = "copyFR:Out"  input="convFR_R:In"  }
        { output = "copyFR:Out"  input="convFR_L:In"  }
        { output = "copySR:Out"  input="convSR_R:In"  }
        { output = "copySR:Out"  input="convSR_L:In"  }
        { output = "copyRR:Out"  input="convRR_R:In"  }
        { output = "copyRR:Out"  input="convRR_L:In"  }
        { output = "copyFC:Out"  input="convFC_R:In"  }
        { output = "copyLFE:Out" input="convLFE_L:In" }
        { output = "copyLFE:Out" input="convLFE_R:In" }

        # output
        { output = "convFL_L:Out"  input="mixL:In 1" }
        { output = "convFL_R:Out"  input="mixR:In 1" }
        { output = "convSL_L:Out"  input="mixL:In 2" }
        { output = "convSL_R:Out"  input="mixR:In 2" }
        { output = "convRL_L:Out"  input="mixL:In 3" }
        { output = "convRL_R:Out"  input="mixR:In 3" }
        { output = "convFC_L:Out"  input="mixL:In 4" }
        { output = "convFC_R:Out"  input="mixR:In 4" }
        { output = "convFR_R:Out"  input="mixR:In 5" }
        { output = "convFR_L:Out"  input="mixL:In 5" }
        { output = "convSR_R:Out"  input="mixR:In 6" }
        { output = "convSR_L:Out"  input="mixL:In 6" }
        { output = "convRR_R:Out"  input="mixR:In 7" }
        { output = "convRR_L:Out"  input="mixL:In 7" }
        { output = "convLFE_R:Out" input="mixR:In 8" }
        { output = "convLFE_L:Out" input="mixL:In 8" }
    ]
    inputs  = [ "copyFL:In" "copyFR:In" "copyFC:In" "copyLFE:In" "copyRL:In" "copyRR:In", "copySL:In", "copySR:In" ]
    outputs = [ "mixL:Out" "mixR:Out" ]
}
    )";

    // Set capture properties for virtual surround module
    const char *capture_props = R"(
{
    node.name = "effect_input.virtual-surround-manager"
    media.class = "Audio/Sink"
    audio.channels = 8
    audio.position = [ FL FR FC LFE RL RR SL SR ]
}
    )";

    // Set playback properties for virtual surround module
    const char *playback_props = R"(
{
    node.name = "effect_output.virtual-surround-manager"
    node.passive = true
    audio.channels = 2
    audio.position = [ FL FR ]
}
    )";

    // Combine the properites to a single JSON string
    const string args = string(R"(
{
    node.description = "Virtual Surround Manager Sink",
    media.name = "Virtual Surround Manager Sink",
    filter.graph = )") + string(filter_graph) + string(R"(,
    capture.props = )") + string(capture_props) + string(R"(,
    playback.props = )") + string(playback_props) + string(R"(
}
    )");

    // Acutally load the module
    pw_impl_module *mod = pw_context_load_module(
        context,
        "libpipewire-module-filter-chain",
        args.c_str(),
        NULL
    );

    // Run the main PipeWire loop
    pw_main_loop_run(loop);

    // TODO: Implement GUI
    // TODO: Implement stream rule

    // Clean up PipeWire stuff
    //pw_core_disconnect(core);
    pw_context_destroy(context);
    pw_main_loop_destroy(loop);
    pw_deinit();

    return 0;
}