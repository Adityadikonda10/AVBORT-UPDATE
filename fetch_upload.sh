#!/bin/bash

# Directories
WATCH_FOLDER="DCIM"
RAW_IMAGE_DIR="images/raw"
LAST_RANGE_FILE="last_range.txt"
SERVICE_ACCOUNT_FILE="service_account.json"

# Initialize directories
mkdir -p "$RAW_IMAGE_DIR"

# Function to fetch images using gphoto2
fetch_images() {
  local last_range=$(cat "$LAST_RANGE_FILE" 2>/dev/null || echo "0")
  
  # Detect the connected camera and fetch images
  if gphoto2 --auto-detect | grep -q "Canon\|Nikon\|Sony\|Pentax\|Olympus\|Leica\|Fujifilm"; then
    echo "Camera detected. Fetching images..."
    gphoto2 --get-raw-data "$last_range-" --filename "$RAW_IMAGE_DIR/%f"
    
    # Update the last range
    last_file=$(ls "$RAW_IMAGE_DIR" | sort -V | tail -n 1 | grep -oP '\d+')
    echo "$last_file" > "$LAST_RANGE_FILE"
  else
    echo "No camera connected."
  fi
}

# Function to upload images to Google Drive
upload_to_drive() {
  local file="$1"
  echo "Uploading $file to Google Drive..."
  python3 - <<EOF
import os
from googleapiclient.discovery import build
from google.oauth2 import service_account

SCOPES = ['https://www.googleapis.com/auth/drive']
SERVICE_ACCOUNT_FILE = '$SERVICE_ACCOUNT_FILE'
PARENT_FOLDER_ID = '1jS2ZKF_eA78difWcCjyFpAfVet_ozHEr'

def authenticate():
    creds = service_account.Credentials.from_service_account_file(SERVICE_ACCOUNT_FILE, scopes=SCOPES)
    return build('drive', 'v3', credentials=creds)

def upload_photo(file_path):
    service = authenticate()
    file_metadata = {'name': os.path.basename(file_path), 'parents': [PARENT_FOLDER_ID]}
    media = file_path
    service.files().create(body=file_metadata, media_body=media).execute()

try:
    upload_photo("$file")
    print(f"Uploaded {file} to Google Drive.")
except Exception as e:
    print(f"Failed to upload {file}: {e}")

EOF
}

# Monitor directory for new files using inotifywait
monitor_and_upload() {
  inotifywait -m -e create "$RAW_IMAGE_DIR" | while read path action file; do
    if [[ "$file" =~ \.(jpg|jpeg|png|CR2)$ ]]; then
      upload_to_drive "$RAW_IMAGE_DIR/$file"
    fi
  done
}

# Main logic
fetch_images
monitor_and_upload
