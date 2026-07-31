import os
import cv2

def resize_images(input_dir, output_dir, target_size):
    
    # 出力フォルダ作成
    os.makedirs(output_dir, exist_ok=True)

    valid_ext = ('.png', '.jpg', '.jpeg')

    for filename in os.listdir(input_dir):
        if filename.lower().endswith(valid_ext):
            input_path = os.path.join(input_dir, filename)
            output_path = os.path.join(output_dir, filename)

            img = cv2.imread(input_path)

            if img is None:
                print(f"failed to load image: {filename}")
                continue

            # リサイズ
            resized = cv2.resize(img, target_size, interpolation=cv2.INTER_AREA)

            # 保存
            cv2.imwrite(output_path, resized)
            print(f"saved: {output_path}")

if __name__ == "__main__":
    # ====== 設定 ======
    input_directory = "../photos/webcam_640x480"  # 元画像が入っているディレクトリ
    output_directory = "../photos/webcam_320x240"  # リサイズ後の画像を保存するディレクトリ
    size = (320, 240)  # (width, height)
    # ==================

    resize_images(input_directory, output_directory, size)
