# Package group for AI/ML support
# This package group includes AI/ML frameworks and tools

SUMMARY = "AI/ML Package Group"
DESCRIPTION = "Package group for AI/ML frameworks and tools"
LICENSE = "MIT"

inherit packagegroup

PACKAGES = "\
    packagegroup-ai \
    packagegroup-ai-frameworks \
    packagegroup-ai-tools \
    "

RDEPENDS:packagegroup-ai = "\
    packagegroup-ai-frameworks \
    packagegroup-ai-tools \
    "

# AI/ML Frameworks
RDEPENDS:packagegroup-ai-frameworks = "\
    tensorflow-lite \
    tensorflow-lite-dev \
    onnxruntime \
    onnxruntime-dev \
    tflite \
    tflite-dev \
    tvm \
    tvm-dev \
    "

# AI/ML Tools
RDEPENDS:packagegroup-ai-tools = "\
    python3-tensorflow \
    python3-tensorflow-lite \
    python3-onnx \
    python3-onnxruntime \
    python3-pytorch \
    python3-torchvision \
    opencv \
    opencv-dev \
    opencv-python \
    "

SUMMARY:packagegroup-ai = "AI/ML Package Group"
SUMMARY:packagegroup-ai-frameworks = "AI/ML Frameworks"
SUMMARY:packagegroup-ai-tools = "AI/ML Tools"

# ============================================================================
# End of packagegroup-ai.bb
# ============================================================================
