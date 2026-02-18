import cv2
import numpy as np
import glob
import os

# === カメラキャリブレーション ===

# 設定
CHECKERBOARD = (7,10) # 内側コーナー数
square_size = 0.023    # 25mm = 0.025m

images = glob.glob('../photos/*.jpg')

# 3D点生成
objp = np.zeros((CHECKERBOARD[0]*CHECKERBOARD[1],3), np.float32)
objp[:,:2] = np.mgrid[0:CHECKERBOARD[0],0:CHECKERBOARD[1]].T.reshape(-1,2)
objp *= square_size

objpoints = []
imgpoints = []

# コーナー検出
for fname in images:
    img = cv2.imread(fname)
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

    image_size = gray.shape[::-1]   # (width, height)
    ret, corners = cv2.findChessboardCorners(gray, CHECKERBOARD, None)

    if ret:
        objpoints.append(objp)
        imgpoints.append(corners)

        cv2.drawChessboardCorners(img, CHECKERBOARD, corners, ret)

# キャリブレーション
ret, mtx, dist, rvecs, tvecs = cv2.calibrateCamera(
    objpoints, imgpoints, gray.shape[::-1], None, None)

print("Camera matrix:\n", mtx)
print("Distortion:\n", dist)
print("Reprojection error:", ret)


# === ORB-SLAM3 YAML生成 ===

fx = mtx[0,0]
fy = mtx[1,1]
cx = mtx[0,2]
cy = mtx[1,2]

k1 = dist[0][0]
k2 = dist[0][1]
p1 = dist[0][2]
p2 = dist[0][3]

width = image_size[0]
height = image_size[1]
fps = 30  # 実測値に変更推奨

yaml_content = f"""%YAML:1.0

#--------------------------------------------------------------------------------------------
# Camera Parameters. Adjust them!
#--------------------------------------------------------------------------------------------
File.version: "1.0"

Camera.type: "PinHole"

# Left Camera calibration and distortion parameters (OpenCV)
Camera1.fx: {fx}
Camera1.fy: {fy}
Camera1.cx: {cx}
Camera1.cy: {cy}

# distortion parameters
Camera1.k1: {k1}
Camera1.k2: {k2}
Camera1.p1: {p1}
Camera1.p2: {p2}

# Camera resolution
Camera.width: {width}
Camera.height: {height}

# Camera frames per second 
Camera.fps: {fps}

# Color order of the images (0: BGR, 1: RGB)
Camera.RGB: 1

#--------------------------------------------------------------------------------------------
# ORB Parameters
#--------------------------------------------------------------------------------------------
ORBextractor.nFeatures: 1250
ORBextractor.scaleFactor: 1.2
ORBextractor.nLevels: 8
ORBextractor.iniThFAST: 20
ORBextractor.minThFAST: 7

#--------------------------------------------------------------------------------------------
# Viewer Parameters
#--------------------------------------------------------------------------------------------
Viewer.KeyFrameSize: 0.05
Viewer.KeyFrameLineWidth: 1.0
Viewer.GraphLineWidth: 0.9
Viewer.PointSize: 2.0
Viewer.CameraSize: 0.08
Viewer.CameraLineWidth: 3.0
Viewer.ViewpointX: 0.0
Viewer.ViewpointY: -0.7
Viewer.ViewpointZ: -3.5
Viewer.ViewpointF: 500.0
"""

output_path = "camera_param.yaml"

with open(output_path, "w") as f:
    f.write(yaml_content)

print(f"ORB-SLAM3 Example-style YAML saved to {output_path}")

