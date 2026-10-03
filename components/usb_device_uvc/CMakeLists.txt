# USB device support only exists on these targets. Register an empty component everywhere else,
# so the same source tree builds for every board without moving this component around
set(USB_DEVICE_UVC_TARGETS esp32s2 esp32s3 esp32p4)
if(NOT "${IDF_TARGET}" IN_LIST USB_DEVICE_UVC_TARGETS)
    idf_component_register()
    return()
endif()

idf_component_register(SRCS usb_device_uvc.c
                    INCLUDE_DIRS "include"
                    REQUIRES usb esp_timer)

idf_component_get_property(tusb_lib espressif__tinyusb COMPONENT_LIB)

target_include_directories(${tusb_lib} PUBLIC "${COMPONENT_DIR}/tusb")
target_sources(${tusb_lib} PUBLIC "${COMPONENT_DIR}/tusb/usb_descriptors.c")

include(package_manager)
cu_pkg_define_version(${CMAKE_CURRENT_LIST_DIR})
