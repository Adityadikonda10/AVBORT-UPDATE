#!/bin/bash

# Configuration
RECORD_FILE="camera_sd_records.json"
RAW_IMAGE_DIR="DCIM"
IMAGE_EXTENSIONS="jpg|jpeg|png|cr2"  # Allowed image formats (case insensitive)
CHECK_INTERVAL=10  # Interval to check for new files (in seconds)
CLOUD_FOLDER_ID="1jS2ZKF_eA78difWcCjyFpAfVet_ozHEr"  # Google Drive folder ID
CREDENTIALS_FILE="service_account.json"  # Service account for Drive API

# Initialize record file
initialize_record_file() {
  if [ ! -f "$RECORD_FILE" ]; then
    echo "{}" > "$RECORD_FILE"  # Empty JSON object
  fi
}

# Get camera identifier
get_camera_identifier() {
  gphoto2 --auto-detect | grep -Eo "usb:[0-9]+,[0-9]+" | head -n 1
}

# Get SD card folder
get_sd_card_folder() {
  gphoto2 --list-folders | grep -Eo "store_.*DCIM/[0-9]+"
}

# Fetch new images
fetch_new_images() {
  local camera_id="$1"
  local sd_card="$2"
  local folder="$RAW_IMAGE_DIR/$camera_id/$sd_card"
  mkdir -p "$folder"

  # Get the last recorded image number
  local last_image
  last_image=$(jq -r --arg camera "$camera_id" --arg sd "$sd_card" '.[$camera][$sd] // 0' "$RECORD_FILE")

  # Fetch new images only
  gphoto2 --list-files | grep -E "^[0-9]+.*($IMAGE_EXTENSIONS)" | awk -v last="$last_image" '$1 > last {print $1}' | while read -r img_num; do
    gphoto2 --get-file "$img_num" --filename "$folder/%f.%C"
    echo "$img_num" > "$folder/last_image.txt"
  done
}

# Update record file
update_record_file() {
  local camera_id="$1"
  local sd_card="$2"
  local folder="$RAW_IMAGE_DIR/$camera_id/$sd_card"
  local last_image
  last_image=$(cat "$folder/last_image.txt")

  jq --arg camera "$camera_id" --arg sd "$sd_card" --argjson image "$last_image" '.[$camera][$sd] = $image' "$RECORD_FILE" > tmp.$$.json && mv tmp.$$.json "$RECORD_FILE"
}

# Upload files to Google Drive
upload_to_drive() {
  local file_path="$1"
  local file_name
  file_name=$(basename "$file_path")

  curl -s -X POST \
    -H "Authorization: Bearer $(get_access_token)" \
    -F "metadata={name : '$file_name', parents: ['$CLOUD_FOLDER_ID']};type=application/json;charset=UTF-8" \
    -F "file=@$file_path;type=$(file --mime-type -b $file_path)" \
    "https://www.googleapis.com/upload/drive/v3/files?uploadType=multipart" >/dev/null && rm "$file_path"
}

# Get Google Drive API access token
get_access_token() {
  curl -s -X POST \
    -H "Content-Type: application/json" \
    -d @"$CREDENTIALS_FILE" \
    "https://oauth2.googleapis.com/token" | jq -r '.access_token'
}

# Process and upload images
process_and_upload_images() {
  local folder="$1"

  find "$folder" -type f -regextype posix-extended -iregex ".*\.($IMAGE_EXTENSIONS)" | while read -r file; do
    upload_to_drive "$file" &
  done
  wait
}

# Main loop
initialize_record_file
while true; do
  camera_id=$(get_camera_identifier)
  if [ -z "$camera_id" ]; then
    echo "No camera detected."
    sleep "$CHECK_INTERVAL"
    continue
  fi

  sd_card=$(get_sd_card_folder)
  if [ -z "$sd_card" ]; then
    echo "No SD card detected."
    sleep "$CHECK_INTERVAL"
    continue
  fi

  fetch_new_images "$camera_id" "$sd_card"
  update_record_file "$camera_id" "$sd_card"

  # Process and upload images in parallel
  folder="$RAW_IMAGE_DIR/$camera_id/$sd_card"
  process_and_upload_images "$folder"

  sleep "$CHECK_INTERVAL"
done
