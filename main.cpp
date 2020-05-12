#include <experimental/filesystem>
#include <functional>
#include <iostream>
#include <sstream>
#include <pulse/pulseaudio.h>
#include <pulse/simple.h>

std::string build_sink_args(
    const std::string& name,
    const std::string& pipe,
    const std::string& format,
    int channels, int rate
) {
    std::stringstream ss;
    ss.sync_with_stdio(false);
    ss.clear();
    ss << "sink_name=" << '"' << name << '"';
    ss << " ";
    ss << "file=" << '"' << pipe << '"';
    ss << " ";
    ss << "format=" << '"' << format << '"';
    ss << " ";
    ss << "rate=" << '"' << std::to_string(rate) << '"';
    ss << " ";
    ss << "channels=" << '"' << std::to_string(channels) << '"';
    std::string str = ss.str();
    return str;
}

std::string build_source_args(
    const std::string& name,
    const std::string& pipe,
    const std::string& format,
    int channels, int rate
) {
    std::stringstream ss;
    ss.sync_with_stdio(false);
    ss.clear();

    ss << "source_name=" << '"' << name << '"';
    ss << " ";
    ss << "file=" << '"' << pipe << '"';
    ss << " ";
    ss << "format=" << '"' << format << '"';
    ss << " ";
    ss << "rate=" << std::to_string(rate);
    ss << " ";
    ss << "channels=" << std::to_string(channels);

    std::string str = ss.str();
    return str;
}

void index_callback(pa_context *c, uint32_t idx, void* userdata) {
    *reinterpret_cast<uint32_t*>(userdata) = idx;
}

int pa_load_module_sync(pa_context* context, const std::string& name, const std::string& args) {

    int mod_idx = 0;

    pa_operation* op = pa_context_load_module(
        context,
        name.c_str(),
        args.c_str(),
        index_callback,
        &mod_idx
    );

    while(true) {
        pa_operation_state state = pa_operation_get_state(op);
        if(state == PA_OPERATION_DONE) { break; }
        if(state == PA_OPERATION_CANCELLED) {
            mod_idx = -1;
            break;
        }
    }

    pa_operation_unref(op);
    return mod_idx;
}

int main() {
    pa_threaded_mainloop *mainloop = pa_threaded_mainloop_new();
    pa_threaded_mainloop_start(mainloop);

    auto context = pa_context_new(pa_threaded_mainloop_get_api(mainloop), "Test");
    pa_context_connect(context, nullptr, pa_context_flags::PA_CONTEXT_NOFLAGS, nullptr);

    while(true) {
        pa_context_state state = pa_context_get_state(context);
        if(state == PA_CONTEXT_READY) { break; }
        if(!PA_CONTEXT_IS_GOOD(state)) {
            std::cout << "Failed connecting to Pulseaudio daemon." << std::endl;
            pa_threaded_mainloop_free(mainloop);
            return -1;
        }
    }

    if(!std::experimental::filesystem::exists("/tmp/xow-output")) {
        std::string args = build_sink_args("Xow output", "/tmp/xow-output", "s16le", 2, 48000);
        int idx = pa_load_module_sync(context, "module-pipe-sink", args);

        if(idx >= 0) {
            std::cout << "Added sink " << idx << "." << std::endl;
        } else {
            std::cout << "Add sink failed." << std::endl;
        }
    } else {
        std::cout << "`/tmp/xow-output` exists, skipping..." << std::endl;
    }

    if(!std::experimental::filesystem::exists("/tmp/xow-input")) {
        std::string args = build_source_args("Xow input", "/tmp/xow-input", "s16le", 2, 48000);
        int idx = pa_load_module_sync(context, "module-pipe-source", args);

        if(idx >= 0) {
            std::cout << "Added source " << idx << "." << std::endl;
        } else {
            std::cout << "Add source failed." << std::endl;
        }
    } else {
        std::cout << "`/tmp/xow-input` exists, skipping..." << std::endl;
    }

    pa_context_disconnect(context);
    pa_threaded_mainloop_free(mainloop);

    return 0;
}
