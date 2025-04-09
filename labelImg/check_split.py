import os
from pathlib import Path

def check_image_label_pairs(dataset_dir):
    images_dir = Path(dataset_dir) / 'images'
    labels_dir = Path(dataset_dir) / 'labels'
    
    for split in ['train', 'val']:
        print(f"\nChecking {split} split:")
        img_dir = images_dir / split
        lbl_dir = labels_dir / split
        
        # Get all image files
        img_files = set([f.stem for f in img_dir.glob('*.jpg')] + 
                       [f.stem for f in img_dir.glob('*.jpeg')] +
                       [f.stem for f in img_dir.glob('*.png')])
        
        # Get all label files
        lbl_files = set([f.stem for f in lbl_dir.glob('*.txt')])
        
        # Check for images without labels
        imgs_without_labels = img_files - lbl_files
        if imgs_without_labels:
            print(f"Images without labels: {imgs_without_labels}")
        
        # Check for labels without images
        labels_without_imgs = lbl_files - img_files
        if labels_without_imgs:
            print(f"Labels without images: {labels_without_imgs}")

check_image_label_pairs('/home/jetson/labelImg/dataset')
