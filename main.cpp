// #include <iostream>
// #include <string>
// #include <set>
// #include <vector>
// #include <chrono>
// #include <thread>
// #include <filesystem>
// #include <fstream>
// #include <sstream>
// #include <cstdio>
// #include <cstdlib>
// #include <wiringPi.h>

// const std::set<std::string> CAMERA_COMPANIES = {
//     "Canon", "Nikon", "Sony", "Fujifilm", "Olympus", "Panasonic", "Leica"
// };

// // LED GPIO pins
// const int BLUE_LED_PIN = 0;   // GPIO 17
// const int GREEN_LED_PIN = 2;  // GPIO 27
// const int RED_LED_PIN = 1;    // GPIO 18

// bool is_camera_connected() {
//     std::string command = "gphoto2 --auto-detect";
//     FILE *pipe = popen(command.c_str(), "r");
//     if (!pipe) {
//         std::cerr << "Failed to run command." << std::endl;
//         return false;
//     }

//     char buffer[128];
//     std::string result;
//     while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
//         result += buffer;
//     }
//     pclose(pipe);

//     // Check for any camera company name in the output
//     for (const auto& company : CAMERA_COMPANIES) {
//         if (result.find(company) != std::string::npos) {
//             return true;
//         }
//     }
//     return false;
// }

// void wait_for_camera() {
//     while (!is_camera_connected()) {
//         // Blink red LED (indicating camera is not connected)
//         std::cout << "Camera not connected. Blinking red LED..." << std::endl;
//         digitalWrite(RED_LED_PIN, HIGH); // Turn on red LED
//         std::this_thread::sleep_for(std::chrono::milliseconds(500));
//         digitalWrite(RED_LED_PIN, LOW); // Turn off red LED
//         std::this_thread::sleep_for(std::chrono::milliseconds(500));
//     }
//     // Turn off red LED and turn on blue LED when connected
//     std::cout << "Camera connected. Turning off red LED and turning on blue LED." << std::endl;
//     digitalWrite(RED_LED_PIN, LOW);  // Ensure red LED is off
//     digitalWrite(BLUE_LED_PIN, HIGH); // Turn on blue LED
// }

// std::string get_last_image_serial() {
//     std::ifstream last_range_file("last_range.txt");
//     std::string last_serial;
//     if (last_range_file.is_open()) {
//         std::getline(last_range_file, last_serial);
//         last_range_file.close();
//     }
//     return last_serial;
// }

// void save_last_image_serial(const std::string& serial) {
//     std::ofstream last_range_file("last_range.txt");
//     if (last_range_file.is_open()) {
//         last_range_file << serial;
//         last_range_file.close();
//     }
// }

// std::vector<std::string> list_files() {
//     std::string command = "gphoto2 --list-files";
//     FILE *pipe = popen(command.c_str(), "r");
//     if (!pipe) {
//         std::cerr << "Failed to run command." << std::endl;
//         return {};
//     }

//     char buffer[128];
//     std::vector<std::string> files;
//     while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
//         files.emplace_back(buffer);
//     }
//     pclose(pipe);

//     return files;
// }

// std::vector<std::string> extract_image_info(const std::vector<std::string>& files) {
//     std::vector<std::string> image_serials;
//     for (const auto& line : files) {
//         std::istringstream iss(line);
//         std::string serial, filename;
//         iss >> serial >> filename;
//         if (!filename.empty() && filename.find("IMG_") != std::string::npos) {
//             image_serials.push_back(line); // Store the entire line for downloading
//         }
//     }
//     return image_serials;
// }

// std::vector<std::string> get_new_images(const std::vector<std::string>& current_files, const std::string& last_serial) {
//     std::vector<std::string> new_images;

//     // Validate the last_serial before conversion
//     int last = -1;
//     if (!last_serial.empty()) {
//         try {
//             last = std::stoi(last_serial);
//         } catch (const std::invalid_argument&) {
//             std::cerr << "Invalid last serial number: " << last_serial << std::endl;
//             last = -1;  // Reset to -1 if conversion fails
//         }
//     }

//     for (const auto& file : current_files) {
//         int serial = std::stoi(file.substr(1, file.find(' ') - 1));
//         if (serial > last) {
//             new_images.push_back(file);
//         }
//     }
//     return new_images;
// }

// void download_images(const std::vector<std::string>& new_images, std::string& last_serial) {
//     for (const auto& file : new_images) {
//         std::string serial = file.substr(1, file.find(' ') - 1);
//         std::string command = "gphoto2 --get-raw-data " + serial;
//         if (std::system(command.c_str()) == 0) {
//             std::cout << "Downloaded image with serial: " << serial << std::endl;
//             last_serial = serial; // Update last_serial after successful download
//             save_last_image_serial(last_serial); // Save the latest serial number upon successful download
            
//             // Blink blue LED for image download
//             digitalWrite(BLUE_LED_PIN, HIGH);
//             std::this_thread::sleep_for(std::chrono::milliseconds(500));
//             digitalWrite(BLUE_LED_PIN, LOW);
//         } else {
//             std::cerr << "Failed to download image with serial: " << serial << std::endl;
//         }
//     }
// }

// void move_images_to_dcim() {
//     const std::filesystem::path current_path = std::filesystem::current_path();
//     const std::filesystem::path dcim_path = current_path / "DCIM";

//     // Create DCIM directory if it doesn't exist
//     if (!std::filesystem::exists(dcim_path)) {
//         std::filesystem::create_directory(dcim_path);
//     }

//     for (const auto& entry : std::filesystem::directory_iterator(current_path)) {
//         if (entry.is_regular_file()) {
//             const auto& filename = entry.path().filename();
//             // Move only image files
//             if (filename.string().find(".jpg") != std::string::npos || filename.string().find(".jpeg") != std::string::npos || filename.string().find(".png") != std::string::npos) {
//                 std::filesystem::rename(entry.path(), dcim_path / filename);
//                 std::cout << "Moved image to DCIM: " << filename << std::endl;

//                 // Blink green LED for file moving process
//                 digitalWrite(GREEN_LED_PIN, HIGH);
//                 std::this_thread::sleep_for(std::chrono::milliseconds(500));
//                 digitalWrite(GREEN_LED_PIN, LOW);
//             }
//         }
//     }
// }

// int main() {
//     // Initialize WiringPi
//     wiringPiSetup(); // Initialize WiringPi library
//     pinMode(BLUE_LED_PIN, OUTPUT); // Set blue LED pin as output
//     pinMode(GREEN_LED_PIN, OUTPUT); // Set green LED pin as output
//     pinMode(RED_LED_PIN, OUTPUT);   // Set red LED pin as output

//     wait_for_camera();

//     // Initial check for the latest image serial
//     auto files = list_files();
//     auto image_info = extract_image_info(files);
//     if (!image_info.empty()) {
//         std::string latest_serial = image_info.back().substr(1, image_info.back().find(' ') - 1);
//         save_last_image_serial(latest_serial); // Save the latest serial number upon connection
//     }

//     std::string last_serial = get_last_image_serial();
//     while (true) {
//         files = list_files();
//         image_info = extract_image_info(files);
//         auto new_images = get_new_images(image_info, last_serial);

//         if (!new_images.empty()) {
//             download_images(new_images, last_serial); // Update last_serial reference
//             move_images_to_dcim();
//         } else {
//             std::cout << "No new images to download." << std::endl;
//         }

//         std::this_thread::sleep_for(std::chrono::seconds(2)); // Check every 5 seconds
//     }

//     return 0;
// }


#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <chrono>
#include <thread>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <wiringPi.h> // Include WiringPi library

const std::set<std::string> CAMERA_COMPANIES = {
    "Canon", "Nikon", "Sony", "Fujifilm", "Olympus", "Panasonic", "Leica"
};

// LED GPIO pin definitions
const int RED_LED_PIN = 0;   // GPIO 17
const int GREEN_LED_PIN = 2; // GPIO 27
const int BLUE_LED_PIN = 3;  // GPIO 22

bool is_camera_connected() {
    std::string command = "gphoto2 --auto-detect";
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) {
        std::cerr << "Failed to run command." << std::endl;
        return false;
    }

    char buffer[128];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);

    // Check for any camera company name in the output
    for (const auto& company : CAMERA_COMPANIES) {
        if (result.find(company) != std::string::npos) {
            return true;
        }
    }
    return false;
}

void wait_for_camera() {
    // Blink red LED while waiting for camera
    while (!is_camera_connected()) {
        digitalWrite(RED_LED_PIN, HIGH);  // Turn on red LED
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        digitalWrite(RED_LED_PIN, LOW);   // Turn off red LED
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    // Turn on blue LED when camera is connected
    digitalWrite(BLUE_LED_PIN, HIGH);
    std::cout << "Camera connected. Turning on blue LED." << std::endl;
}

std::string get_last_image_serial() {
    std::ifstream last_range_file("last_range.txt");
    std::string last_serial;
    if (last_range_file.is_open()) {
        std::getline(last_range_file, last_serial);
        last_range_file.close();
    }
    return last_serial;
}

void save_last_image_serial(const std::string& serial) {
    std::ofstream last_range_file("last_range.txt");
    if (last_range_file.is_open()) {
        last_range_file << serial;
        last_range_file.close();
    }
}

std::vector<std::string> list_files() {
    std::string command = "gphoto2 --list-files";
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) {
        std::cerr << "Failed to run command." << std::endl;
        return {};
    }

    char buffer[128];
    std::vector<std::string> files;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        files.emplace_back(buffer);
    }
    pclose(pipe);

    return files;
}

std::vector<std::string> extract_image_info(const std::vector<std::string>& files) {
    std::vector<std::string> image_serials;
    for (const auto& line : files) {
        std::istringstream iss(line);
        std::string serial, filename;
        iss >> serial >> filename;
        if (!filename.empty() && filename.find("IMG_") != std::string::npos) {
            image_serials.push_back(line); // Store the entire line for downloading
        }
    }
    return image_serials;
}

std::vector<std::string> get_new_images(const std::vector<std::string>& current_files, const std::string& last_serial) {
    std::vector<std::string> new_images;

    // Validate the last_serial before conversion
    int last = -1;
    if (!last_serial.empty()) {
        try {
            last = std::stoi(last_serial);
        } catch (const std::invalid_argument&) {
            std::cerr << "Invalid last serial number: " << last_serial << std::endl;
            last = -1;  // Reset to -1 if conversion fails
        }
    }

    for (const auto& file : current_files) {
        int serial = std::stoi(file.substr(1, file.find(' ') - 1));
        if (serial > last) {
            new_images.push_back(file);
        }
    }
    return new_images;
}

void download_images(const std::vector<std::string>& new_images, std::string& last_serial) {
    for (const auto& file : new_images) {
        std::string serial = file.substr(1, file.find(' ') - 1);
        std::string command = "gphoto2 --get-raw-data " + serial;
        if (std::system(command.c_str()) == 0) {
            std::cout << "Downloaded image with serial: " << serial << std::endl;
            last_serial = serial; // Update last_serial after successful download
            save_last_image_serial(last_serial); // Save the latest serial number upon successful download
            
            // Blink blue LED after downloading
            digitalWrite(BLUE_LED_PIN, HIGH);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            digitalWrite(BLUE_LED_PIN, LOW);
        } else {
            std::cerr << "Failed to download image with serial: " << serial << std::endl;
        }
    }
}

void move_images_to_dcim() {
    const std::filesystem::path current_path = std::filesystem::current_path();
    const std::filesystem::path dcim_path = current_path / "DCIM";

    // Create DCIM directory if it doesn't exist
    if (!std::filesystem::exists(dcim_path)) {
        std::filesystem::create_directory(dcim_path);
    }

    for (const auto& entry : std::filesystem::directory_iterator(current_path)) {
        if (entry.is_regular_file()) {
            const auto& filename = entry.path().filename();
            // Move only image files
            if (filename.string().find(".jpg") != std::string::npos || filename.string().find(".jpeg") != std::string::npos || filename.string().find(".png") != std::string::npos) {
                std::filesystem::rename(entry.path(), dcim_path / filename);
                std::cout << "Moved image to DCIM: " << filename << std::endl;
                
                // Blink green LED after moving
                digitalWrite(GREEN_LED_PIN, HIGH);
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                digitalWrite(GREEN_LED_PIN, LOW);
            }
        }
    }
}

int main() {
    // Initialize WiringPi and set LED pins as output
    wiringPiSetup(); // Use the WiringPi setup
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(BLUE_LED_PIN, OUTPUT);
    
    // Ensure all LEDs are off at startup
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(BLUE_LED_PIN, LOW);

    wait_for_camera();

    // Initial check for the latest image serial
    auto files = list_files();
    auto image_info = extract_image_info(files);
    if (!image_info.empty()) {
        std::string latest_serial = image_info.back().substr(1, image_info.back().find(' ') - 1);
        save_last_image_serial(latest_serial); // Save the latest serial number upon connection
    }

    std::string last_serial = get_last_image_serial();
    while (true) {
        files = list_files();
        image_info = extract_image_info(files);
        auto new_images = get_new_images(image_info, last_serial);

        if (!new_images.empty()) {
            download_images(new_images, last_serial); // Update last_serial reference
            move_images_to_dcim();
        } else {
            std::cout << "No new images to download." << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(5)); // Check every 5 seconds
    }

    return 0;
}
