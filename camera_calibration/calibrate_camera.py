import cv2
import numpy as np
import glob
import os
import sys

# === カメラキャリブレーション ===

# 設定
CHECKERBOARD = (7, 10)  # 内側コーナー数 (cols, rows)
square_size = 23.0      # mm 単位

# 画像パターン（スクリプトの場所からの相対パス）
PHOTO_DIR = "../photos/webcam_320x240"  # キャリブレーション画像が入っているディレクトリ
OUTUPT_FILE = "webcam_320x240_param.yaml"

images = glob.glob(os.path.join(PHOTO_DIR, '*.png')) + glob.glob(os.path.join(PHOTO_DIR, '*.jpg')) + glob.glob(os.path.join(PHOTO_DIR, '*.jpeg'))

if len(images) == 0:
    print(f'No images found in {PHOTO_DIR}. Please check the path and add calibration photos.')
    sys.exit(1)

# 3D点生成 (CHECKERBOARD の内側コーナー数に合わせる)
objp = np.zeros((CHECKERBOARD[0] * CHECKERBOARD[1], 3), np.float32)
objp[:, :2] = np.mgrid[0:CHECKERBOARD[0], 0:CHECKERBOARD[1]].T.reshape(-1, 2)
objp *= square_size

objpoints = []
imgpoints = []
image_size = None

# 角点検出の基準
criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)

for fname in images:
    img = cv2.imread(fname)
    if img is None:
        print(f'Warning: could not read image {fname}, skipping')
        continue
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

    if image_size is None:
        image_size = gray.shape[::-1]  # (width, height)

    flags = cv2.CALIB_CB_ADAPTIVE_THRESH | cv2.CALIB_CB_NORMALIZE_IMAGE
    ret, corners = cv2.findChessboardCorners(gray, CHECKERBOARD, flags)

    if ret:
        # 角点をサブピクセル精度に補正
        corners_sub = cv2.cornerSubPix(gray, corners, (11, 11), (-1, -1), criteria)
        objpoints.append(objp.copy())
        imgpoints.append(corners_sub)
        cv2.drawChessboardCorners(img, CHECKERBOARD, corners_sub, ret)
    else:
        print(f'Chessboard not found in {fname}')

if len(objpoints) == 0:
    print('No valid chessboard detections; check CHECKERBOARD and images.')
    sys.exit(1)

# キャリブレーション
ret, mtx, dist, rvecs, tvecs = cv2.calibrateCamera(
    objpoints, imgpoints, image_size, None, None)

print('Camera matrix:\n', mtx)
print('Distortion:\n', dist.ravel())

# 再投影誤差を厳密に計算
tot_error = 0
total_points = 0
for i in range(len(objpoints)):
    imgpoints2, _ = cv2.projectPoints(objpoints[i], rvecs[i], tvecs[i], mtx, dist)
    error = cv2.norm(imgpoints[i], imgpoints2, cv2.NORM_L2)
    tot_error += error ** 2
    total_points += len(imgpoints2)
mean_error = np.sqrt(tot_error / total_points)
print('Reprojection RMS (calibrateCamera returned):', ret)
print('Reprojection mean error (per-point):', mean_error)


# === ORB-SLAM3 YAML生成 ===

fx = float(mtx[0, 0])
fy = float(mtx[1, 1])
cx = float(mtx[0, 2])
cy = float(mtx[1, 2])

# dist は (1,5) か (k,1,5) などの形状なのでフラットにして取得
dist_flat = dist.ravel()
k1 = float(dist_flat[0]) if dist_flat.size > 0 else 0.0
k2 = float(dist_flat[1]) if dist_flat.size > 1 else 0.0
p1 = float(dist_flat[2]) if dist_flat.size > 2 else 0.0
p2 = float(dist_flat[3]) if dist_flat.size > 3 else 0.0
k3 = float(dist_flat[4]) if dist_flat.size > 4 else 0.0

width = int(image_size[0])
height = int(image_size[1])
fps = 20  # 実測値に変更推奨

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
Camera1.k3: {k3}

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
ORBextractor.nFeatures: 2000
ORBextractor.scaleFactor: 1.2
ORBextractor.nLevels: 8
ORBextractor.iniThFAST: 10
ORBextractor.minThFAST: 5

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


with open(OUTUPT_FILE, 'w') as f:
    f.write(yaml_content)

print(f'ORB-SLAM3 Example-style YAML saved to {OUTUPT_FILE}')
