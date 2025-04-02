import os
import sys
import subprocess
import shutil
import urllib.request
import zipfile
import tarfile
import platform as pf

def run_command(cmd):
    print(f"执行命令: {cmd}")
    subprocess.run(cmd, shell=True, check=True)

def download_file(url, filename):
    print(f"下载文件: {url}")
    urllib.request.urlretrieve(url, filename)

def setup_godot_cpp():
    print("设置 godot-cpp...")
    if not os.path.exists("godot-cpp"):
        run_command("git submodule add https://github.com/godotengine/godot-cpp.git")
        run_command("git submodule update --init --recursive")
        # 不再尝试构建godot-cpp，这将在GitHub Actions工作流中单独处理

def setup_opencv_mobile():
    print("设置 OpenCV-mobile...")
    opencv_dir = "thirdparty/opencv-mobile"
    os.makedirs(opencv_dir, exist_ok=True)
    
    # 获取构建平台
    build_platform = get_build_platform()
    
    # 下载预编译的OpenCV-mobile
    if build_platform == "windows":
        url = "https://github.com/nihui/opencv-mobile/releases/latest/download/opencv-mobile-4.11.0-windows-vs2022.zip"
    elif build_platform == "ios":
        url = "https://github.com/nihui/opencv-mobile/releases/download/v32/opencv-mobile-4.11.0-ios.zip"
    elif build_platform == "macos":
        # 对于macOS，我们也使用iOS版本，然后做必要的调整
        url = "https://github.com/nihui/opencv-mobile/releases/download/v32/opencv-mobile-4.11.0-macos.zip"
    else:  # android
        url = "https://github.com/nihui/opencv-mobile/releases/download/v32/opencv-mobile-4.11.0-android.zip"
    
    download_file(url, "opencv-mobile.zip")
    
    with zipfile.ZipFile("opencv-mobile.zip", "r") as zip_ref:
        zip_ref.extractall(opencv_dir)
    
    # 为macOS创建专门的库目录
    if build_platform == "macos":
        os.makedirs(os.path.join(opencv_dir, "lib", "macos"), exist_ok=True)
        for file in os.listdir(os.path.join(opencv_dir, "lib", "ios")):
            if file.endswith(".a") or file.endswith(".dylib"):
                shutil.copy(
                    os.path.join(opencv_dir, "lib", "ios", file),
                    os.path.join(opencv_dir, "lib", "macos")
                )
    
    os.remove("opencv-mobile.zip")

def setup_litert():
    print("设置 LiteRT...")
    litert_dir = "thirdparty/litert"
    os.makedirs(litert_dir, exist_ok=True)
    
    # 获取构建平台
    build_platform = get_build_platform()
    
    # 克隆LiteRT仓库
    if not os.path.exists(os.path.join(litert_dir, ".git")):
        run_command(f"git clone https://github.com/google-ai-edge/LiteRT.git {litert_dir}")
        os.chdir(litert_dir)
        
        # 初始化子模块
        run_command("git submodule init")
        run_command("git submodule update --remote")
        
        # 创建lib目录
        os.makedirs("lib", exist_ok=True)
        
        # 直接使用系统安装的Bazel构建，不使用Docker
        if build_platform == "windows":
            # Windows平台使用Bazel构建
            build_cmd = "bazel build //tflite:libtensorflowlite.so --config=windows"
            run_command(build_cmd)
            shutil.copy("bazel-bin/tflite/libtensorflowlite.so", "lib/libtensorflowlite.dll")
            
        elif build_platform == "ios":
            # iOS平台使用Bazel构建
            build_cmd = "bazel build //tflite:libtensorflowlite.so --config=ios"
            run_command(build_cmd)
            shutil.copy("bazel-bin/tflite/libtensorflowlite.so", "lib/libtensorflowlite.dylib")
            
        elif build_platform == "macos":
            # macOS平台使用Bazel构建
            build_cmd = "bazel build //tflite:libtensorflowlite.so --config=macos"
            run_command(build_cmd)
            shutil.copy("bazel-bin/tflite/libtensorflowlite.so", "lib/libtensorflowlite.dylib")
            
        else:  # android
            # Android平台使用Bazel构建
            build_cmd = "bazel build //tflite:libtensorflowlite.so --config=android"
            run_command(build_cmd)
            shutil.copy("bazel-bin/tflite/libtensorflowlite.so", "lib/libtensorflowlite.so")
        
        os.chdir("..")

def get_build_platform():
    """根据命令行参数或当前系统确定构建平台"""
    # 检查是否通过命令行指定了平台
    for i, arg in enumerate(sys.argv):
        if arg == '--platform' and i + 1 < len(sys.argv):
            return sys.argv[i + 1]
        elif arg.startswith('--platform='):
            return arg.split('=')[1]
    
    # 如果没有指定，则根据系统推断
    if sys.platform == 'win32':
        return 'windows'
    elif sys.platform == 'darwin':
        # 检测是否是macOS或iOS构建
        # 可以根据其他环境变量或配置文件来决定
        # 默认为macOS
        return 'macos'
    else:
        return 'android'

def main():
    print("开始设置依赖...")
    setup_godot_cpp()
    setup_opencv_mobile()
    setup_litert()
    print("依赖设置完成！")

if __name__ == "__main__":
    main() 