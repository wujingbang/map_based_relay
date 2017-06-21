# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')
def configure(conf):
    conf.env.append_value("CXXFLAGS", ["-fPIC"])
    conf.env.append_value("CFLAGS", ["-fPIC"])
    conf.env.append_value("LINKFLAGS", ["-L/home/wu/workspace/ns-3/ns-3.26/src/wave/model/proj/lib"])
    conf.env.append_value("LIB", ["proj"])

def build(bld):
    module = bld.create_ns3_module('wave', ['core','wifi', 'propagation', 'internet'])
    module.source = [
        'model/wave-mac-low.cc',
        'model/ocb-wifi-mac.cc',
        'model/vendor-specific-action.cc',
        'model/channel-coordinator.cc',
        'model/channel-scheduler.cc',
        'model/default-channel-scheduler.cc',
        'model/channel-manager.cc',
        'model/vsa-manager.cc',
        'model/bsm-application.cc',
        'model/higher-tx-tag.cc',
        'model/wave-net-device.cc',
        'helper/wave-bsm-stats.cc',
        'helper/wave-mac-helper.cc',
        'helper/wave-helper.cc',
        'helper/wifi-80211p-helper.cc',
        'helper/wave-bsm-helper.cc',
        'model/debug.c',
        'model/geohash.c',
        'model/graph.c',
        'model/list.c',
        'model/mbr_route.c',
        'model/mbr_sumomap.cpp',
        'model/neighbors.c',
        'model/tinyxml2.cpp',
        'model/utils.c',
        'model/mbr-beacon.cpp',
        ]

    module_test = bld.create_ns3_module_test_library('wave')
    module_test.source = [
        'test/mac-extension-test-suite.cc',
        'test/ocb-test-suite.cc',
        ]

    headers = bld(features='ns3header')
    headers.module = 'wave'
    headers.source = [
        'model/wave-mac-low.h',
        'model/ocb-wifi-mac.h',
        'model/vendor-specific-action.h',
        'model/channel-coordinator.h',
        'model/channel-manager.h',
        'model/channel-scheduler.h',
        'model/default-channel-scheduler.h',
        'model/vsa-manager.h',
        'model/higher-tx-tag.h',
        'model/wave-net-device.h',
        'model/bsm-application.h',
        'helper/wave-bsm-stats.h',
        'helper/wave-mac-helper.h',
        'helper/wave-helper.h',
        'helper/wifi-80211p-helper.h',
        'helper/wave-bsm-helper.h',
        'model/common.h',
        'model/debug.h',
        'model/geohash.h',
        'model/graph.h',
        'model/linux_list.h',
        'model/list.h',
        'model/mbr.h',
        'model/mbr_route.h',
        'model/mbr_sumomap.h',
        'model/neighbors.h',
        'model/tinyxml2.h',
        'model/utils.h',
        'model/mbr-beacon.h',
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.recurse('examples')

    bld.ns3_python_bindings()

