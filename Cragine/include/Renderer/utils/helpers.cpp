#include "helpers.h"
#include "utils/Logger.h"
#include <webgpu/webgpu.hpp>

namespace crg::renderer::helpers {

    wgpu::Adapter requestAdapterSync(wgpu::Instance instance, wgpu::RequestAdapterOptions const * options) {
    	// A simple structure holding the local information shared with the
    	// onAdapterRequestEnded callback.
    	struct UserData {
    		wgpu::Adapter adapter = nullptr;
    		bool requestEnded = false;
    	};
    	UserData userData;

    	// Callback called by wgpuInstanceRequestAdapter when the request returns
    	// This is a C++ lambda function, but could be any function defined in the
    	// global scope. It must be non-capturing (the brackets [] are empty) so
    	// that it behaves like a regular C function pointer, which is what
    	// wgpuInstanceRequestAdapter expects (WebGPU being a C API). The workaround
    	// is to convey what we want to capture through the pUserData pointer,
    	// provided as the last argument of wgpuInstanceRequestAdapter and received
    	// by the callback as its last argument.
    	auto onAdapterRequestEnded = [](
            WGPURequestAdapterStatus status,
            WGPUAdapter adapter,
            WGPUStringView message,
            void * pUserData1,
            void * /*pUserData2*/
        ) {
    		UserData& userData = *reinterpret_cast<UserData*>(pUserData1);
    		if (status == WGPURequestAdapterStatus_Success) {
    			userData.adapter = adapter;
    		} else {
                std::string msg(message.data, message.length);
    			LOG_CORE_ERROR("Could not get WebGPU adapter: {}", msg);
    		}
    		userData.requestEnded = true;
    	};

        wgpu::RequestAdapterCallbackInfo callbackInfo{};
        callbackInfo.nextInChain = nullptr;
        callbackInfo.mode = WGPUCallbackMode_WaitAnyOnly;
        callbackInfo.callback = onAdapterRequestEnded;
        callbackInfo.userdata1 = &userData;


    	// Call to the WebGPU request adapter procedure
    	wgpuInstanceRequestAdapter(
    		instance /* equivalent of navigator.gpu */,
    		options,
    		callbackInfo
    	);

    	// We wait until userData.requestEnded gets true
        #ifdef __EMSCRIPTEN__
    		while (!userData.requestEnded) {
    			emscripten_sleep(100);
    		}
        #endif // __EMSCRIPTEN__

    	assert(userData.requestEnded);

    	return userData.adapter;
    }



    /**
     * Utility function to get a WebGPU device, so that
     *     WGPUDevice device = requestDeviceSync(adapter, options);
     * is roughly equivalent to
     *     const device = await adapter.requestDevice(descriptor);
     * It is very similar to requestAdapter
     */
    wgpu::Device requestDeviceSync(wgpu::Adapter adapter, wgpu::DeviceDescriptor const * descriptor) {
        struct UserData {
            wgpu::Device device = nullptr;
            bool requestEnded = false;
        };
        UserData userData;

        auto onDeviceRequestEnded = [](
            WGPURequestDeviceStatus status,
            WGPUDevice device,
            WGPUStringView message,
            void * pUserData1,
            void * /*pUserData2*/
        ) {
            UserData& userData = *reinterpret_cast<UserData*>(pUserData1);
            if (status == WGPURequestDeviceStatus_Success) {
                userData.device = device;
            } else {
                LOG_CORE_ERROR("Could not get WebGPU device: {}", message.data);
            }
            userData.requestEnded = true;
        };

        wgpu::RequestDeviceCallbackInfo callbackInfo{};
        callbackInfo.nextInChain = nullptr;
        callbackInfo.mode = WGPUCallbackMode_WaitAnyOnly;
        callbackInfo.callback = onDeviceRequestEnded;
        callbackInfo.userdata1 = &userData;

        wgpuAdapterRequestDevice(
            adapter,
            descriptor,
            callbackInfo
        );

        #ifdef __EMSCRIPTEN__
            while (!userData.requestEnded) {
                emscripten_sleep(100);
            }
        #endif // __EMSCRIPTEN__

        assert(userData.requestEnded);

        return userData.device;
    }

}
