import os
import sys
from SCons.Script import *

# 获取命令行参数
AddOption('--platform',
        dest='platform',
        type='string',
        nargs=1,
        action='store',
        help='指定目标平台: windows, macos, ios, android')

platform = GetOption('platform')

if platform is None:
    if sys.platform == 'win32':
        platform = 'windows'
    elif sys.platform == 'darwin':
        platform = 'macos'
    elif sys.platform == 'linux':
        platform = 'android'

print(f"构建平台: {platform}")

# 设置环境变量
env = Environment(
    CXXFLAGS = ['-std=c++17', '-fPIC'],
    CPPPATH = [
        '#include',
        '#godot-cpp/include',
        '#godot-cpp/gen/include',
        '#thirdparty/opencv-mobile/include',
        '#thirdparty/litert/include'
    ]
)

# 添加godot-cpp库路径
godot_cpp_lib_path = 'godot-cpp/bin'
godot_lib_name = 'godot-cpp'

# 根据平台设置编译选项
if platform == 'windows':
    env.Append(CPPDEFINES=['WIN32'])
    env.Append(LIBPATH=[
        '#thirdparty/opencv-mobile/lib/windows', 
        '#thirdparty/litert/lib',
        f'#{godot_cpp_lib_path}'
    ])
    env.Append(LIBS=['opencv_mobile', 'libtensorflowlite', godot_lib_name + '.windows.template_release.x86_64'])
elif platform == 'ios':
    env.Append(CPPDEFINES=['__APPLE__', 'IOS_PLATFORM'])
    env.Append(FRAMEWORKPATH=['#thirdparty/opencv-mobile/lib/ios'])
    env.Append(LIBPATH=[
        '#thirdparty/litert/lib',
        f'#{godot_cpp_lib_path}'
    ])
    env.Append(LIBS=['opencv_mobile', 'libtensorflowlite', godot_lib_name + '.ios.template_release.universal'])
elif platform == 'macos':
    env.Append(CPPDEFINES=['__APPLE__', 'MACOS_PLATFORM'])
    env.Append(FRAMEWORKPATH=['#thirdparty/opencv-mobile/lib/macos'])
    env.Append(LIBPATH=[
        '#thirdparty/litert/lib',
        f'#{godot_cpp_lib_path}'
    ])
    env.Append(LIBS=['opencv_mobile', 'libtensorflowlite', godot_lib_name + '.macos.template_release.universal'])
elif platform == 'android':
    env.Append(CPPDEFINES=['__ANDROID__'])
    env.Append(LIBPATH=[
        '#thirdparty/opencv-mobile/lib/android', 
        '#thirdparty/litert/lib',
        f'#{godot_cpp_lib_path}'
    ])
    env.Append(LIBS=['opencv_mobile', 'libtensorflowlite', godot_lib_name + '.android.template_release.arm64'])

# 源文件
sources = Glob('src/*.cpp')

# 根据平台设置输出目标
if platform == 'windows':
    target = env.SharedLibrary('bin/windows/DiffDetectorGDExtension', sources)
elif platform == 'ios':
    target = env.SharedLibrary('bin/ios/libDiffDetectorGDExtension', sources)
elif platform == 'macos':
    target = env.SharedLibrary('bin/macos/libDiffDetectorGDExtension', sources)
elif platform == 'android':
    target = env.SharedLibrary('bin/android/libDiffDetectorGDExtension', sources)

# 默认目标
Default(target) 