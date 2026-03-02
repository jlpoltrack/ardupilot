# encoding: utf-8

"""
waf tool for RP2350 build
handles the hybrid waf -> cmake -> pico sdk build pipeline
"""

from waflib import Build, ConfigSet, Configure, Context, Task, Utils
from waflib import Errors, Logs
from waflib.TaskGen import before, after_method, before_method, feature
from waflib.Configure import conf
from collections import OrderedDict

import os
import sys

def configure(cfg):
    bldnode = cfg.bldnode.make_node(cfg.variant)

    def srcpath(path):
        return cfg.srcnode.make_node(path).abspath()

    def bldpath(path):
        return bldnode.make_node(path).abspath()

    # load cmake builder
    cfg.load('cmake')

    # define env and location for the cmake rp2350 file
    env = cfg.env
    env.AP_HAL_RP2350 = srcpath('libraries/AP_HAL_RP2350/targets/rp2350')
    env.AP_PROGRAM_FEATURES += ['rp2350_ap_program']

    env.RP2350_PREFIX_REL = 'rp2350-build'
    env.BUILDROOT = bldpath('')
    env.SRCROOT = srcpath('')

    env.PICO_SDK_PATH = os.environ.get(
        'PICO_SDK_PATH',
        srcpath('modules/pico-sdk')
    )
    env.FREERTOS_KERNEL_PATH = os.environ.get(
        'FREERTOS_KERNEL_PATH',
        srcpath('modules/FreeRTOS-Kernel')
    )

    # generate hwdef.h from hwdef.dat
    hwdef_dir = os.path.join(env.SRCROOT, 'libraries/AP_HAL_RP2350/hwdef')
    if len(env.HWDEF) == 0:
        env.HWDEF = os.path.join(hwdef_dir, env.BOARD, 'hwdef.dat')
    # for phase 1, hwdef.h is hand-maintained (no script generation needed)


def pre_build(self):
    """configure cmake build for rp2350"""
    lib_vars = OrderedDict()
    lib_vars['ARDUPILOT_CMD'] = self.cmd
    lib_vars['WAF_BUILD_TARGET'] = self.targets
    lib_vars['ARDUPILOT_LIB'] = self.bldnode.find_or_declare('lib/').abspath()
    lib_vars['ARDUPILOT_BIN'] = self.bldnode.find_or_declare('lib/bin').abspath()
    lib_vars['PICO_SDK_PATH'] = self.env.PICO_SDK_PATH
    lib_vars['FREERTOS_KERNEL_PATH'] = self.env.FREERTOS_KERNEL_PATH

    rp2350_build = self.cmake(
        name='rp2350-sdk',
        cmake_vars=lib_vars,
        cmake_src='libraries/AP_HAL_RP2350/targets/rp2350',
        cmake_bld='rp2350-build',
    )


@feature('rp2350_ap_program')
@after_method('process_source')
def rp2350_firmware(self):
    self.link_task.always_run = True
    rp2350_build = self.bld.cmake('rp2350-sdk')

    build = rp2350_build.build('all', target='rp2350-build/ardupilot.uf2')
    build.post()

    build.cmake_build_task.set_run_after(self.link_task)
