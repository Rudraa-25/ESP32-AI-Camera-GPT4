🤖 ESP32 AI Camera with GPT-4o (OpenAI Vision)
A compact AI-powered camera prototype built using ESP32-CAM, capable of capturing images and sending them to ChatGPT-4o for intelligent visual analysis. It displays a summarized output on a 0.96" OLED display.

📸 Features
📷 Captures images via ESP32-CAM (OV2640)

🌐 Connects to WiFi & sends image in Base64 to ChatGPT-4o

🧠 Uses OpenAI GPT-4o to generate summary of the image

🖥️ Displays response on OLED screen

🔘 Single-button capture & auto summarization

🔊 Optional buzzer feedback

🔋 Portable setup powered by battery & TP4056 charging module

🧰 Hardware Used
Component	Description
ESP32-CAM	AI-Thinker variant with OV2640 camera
0.96" OLED Display	SSD1306, I2C (128x64)
TP4056 Module	Li-ion Battery charging module
MT3608 Boost	3.7V to 5V converter
Push Button	For triggering capture manually
Buzzer (Optional)	Feedback tone
18650 Battery	Power Source

🔌 Pin Connections
Module	ESP32 Pin
OLED SDA	GPIO 15
OLED SCL	GPIO 14
Button	GPIO 13
Buzzer	GPIO 2

Make sure your OLED is set to 0x3C I2C address

🧠 OpenAI Integration
Model: gpt-4o

API Call: Sends image as base64 in image_url field using "data:image/jpeg;base64,..."

Custom prompt: e.g., "Summarize the image" or "Describe the scene"

💻 How It Works
ESP32 connects to WiFi

On button press, captures an image

Converts image to Base64

Sends request to OpenAI API with the image

Receives summarized response

Displays it on OLED in scrollable form

🔋 Power Setup
Module	Connection
TP4056 B+/-	18650 Battery terminals
TP4056 OUT+/-	MT3608 IN+/-
MT3608 OUT+	ESP32 5V
MT3608 OUT-	ESP32 GND

🧪 Future Improvements
📦 Store image + response in SD card

🧩 Add voice output (TTS)

📶 Enable OTA updates

🧠 Add offline ML support (with tinyML)

📷 Demo Reel Ideas
POV: Everyone’s asleep and you’re making a camera talk to ChatGPT 😤
✨ Tagline: "Built with 4 wires, 1 dream, no sleep."

🚀 How to Use
Flash the code from /Arduino_Code to your ESP32-CAM

Replace your OpenAI API key in the code

Connect OLED + button as per pinout

Power using 5V or battery setup

Press the button to capture & see the magic ✨

📦 Folder Structure
bash
Copy
Edit
ESP32-AI-Camera-GPT4/
├── Arduino_Code/
│   └── FInal_Project_AI_camara.ino
├── README.md
├── LICENSE
└── .gitignore
📄 License
This project is licensed under the MIT License.

