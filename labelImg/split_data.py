import os
import shutil
import random

def split_data(image_dir, label_dir, output_base, split_ratios=[0.8, 0.1, 0.1]):
    """
    Split the dataset into train/val/test sets
    """
    # Get all image files
    image_files = [f for f in os.listdir(image_dir) if f.endswith(('.jpg', '.jpeg', '.png'))]
    random.shuffle(image_files)
    
    # Calculate split points
    n = len(image_files)
    train_end = int(n * split_ratios[0])
    val_end = train_end + int(n * split_ratios[1])
    
    # Split files
    train_files = image_files[:train_end]
    val_files = image_files[train_end:val_end]
    test_files = image_files[val_end:]
    
    # Helper function to copy files
    def copy_files(files, subset):
        for f in files:
            # Copy image
            src_img = os.path.join(image_dir, f)
            dst_img = os.path.join(output_base, 'images', subset, f)
            shutil.copy2(src_img, dst_img)
            
            # Copy corresponding label
            label_file = os.path.splitext(f)[0] + '.txt'
            src_label = os.path.join(label_dir, label_file)
            dst_label = os.path.join(output_base, 'labels', subset, label_file)
            if os.path.exists(src_label):  # Only copy if label exists
                shutil.copy2(src_label, dst_label)
    
    # Copy files to respective directories
    copy_files(train_files, 'train')
    copy_files(val_files, 'val')
    copy_files(test_files, 'test')

# Use the function
split_data(
    'images',  # Directory containing your images
    'labels',  # Directory containing your label txt files
    'dataset'      # Base directory for the organized dataset
)
