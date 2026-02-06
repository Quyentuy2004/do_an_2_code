
#include <Wire.h>
//#include <LiquidCrystal_I2C.h>

#define DHTPIN 4  // Chân GPIO4 kết nối với chân DATA của cảm biến DHT22
#define LEDWAR 23  
#define COIWAR 5

//LiquidCrystal_I2C lcd(0x27, 16, 2);



// Biến lưu dữ liệu thô từ DHT22
byte RH_high, RH_low, temp_high, temp_low, checksum;
float hum;   // Giá trị độ ẩm sau khi tính toán
float temp;  // Giá trị nhiệt độ sau khi tính toán
unsigned long startTime;  // Biến thời gian để đo độ dài xung


// void setup_lcd(){
//     Wire.begin(21,22);
//   lcd.init();              // Khởi động LCD
//   lcd.backlight();         // Bật đèn nền
 
//   lcd.setCursor(0, 0);
//   lcd.print("Dang khoi dong...");
//    delay(1000);
// }

// Đọc 1 byte dữ liệu từ DHT22
byte Readbit(){
  byte dataByte = 0;
  for(byte i = 0; i < 8; i++)  // Lặp 8 lần để đọc 8 bit
  {
    while(digitalRead(DHTPIN) == LOW);  // Bắt đầu mỗi bit dht đều kéo xuống LOW  khoảng 50us để chờ đến khi DHT kéo HIGH
    // Bắt đầu đo độ dài xung HIGH để xác định là bit 0 hay 1
    startTime = micros();
    while (digitalRead(DHTPIN) == HIGH);  // Đợi đến khi DHT kéo LOW
    // Nếu thời gian xung HIGH > 40us thì là bit 1, ngược lại là bit 0
    if (micros() - startTime > 40) {
      dataByte = (dataByte << 1) | 0x01;  // Dịch trái và thêm bit 1
    } else {
      dataByte = (dataByte << 1);         // Dịch trái và thêm bit 0
    }
  }
  return dataByte;  // Trả về 1 byte dữ liệu
}

// Đọc toàn bộ dữ liệu từ DHT22, trả về true nếu thành công
bool readDHT22() {
  // Reset các giá trị trước khi đọc mới
  RH_high = 0;
  RH_low = 0;
  temp_high = 0;
  temp_low = 0;
  checksum = 0;

  // Gửi tín hiệu bắt đầu tới DHT22
  pinMode(DHTPIN, OUTPUT);          //khai báo chân DHTPIN là OUTPUT
  digitalWrite(DHTPIN, LOW);          
  delayMicroseconds(18000);         // Kéo thấp chân DATA trong 18ms
  digitalWrite(DHTPIN, HIGH);       
  delayMicroseconds(35);            //// Kéo lên cao trong 35us
  pinMode(DHTPIN, INPUT);           // Đổi sang chế độ INPUT để nhận dữ liệu từ DHT22

  // Chờ phản hồi của DHT22: đầu tiên là tín hiệu LOW khoảng 80us
  startTime = micros();
  while (digitalRead(DHTPIN) == HIGH) {
    if (micros() - startTime > 100) return false;  // Timeout nếu quá 100us
  }

  // Sau đó là tín hiệu LOW khoảng 80us
  startTime = micros();
  while (digitalRead(DHTPIN) == LOW) {
    if (micros() - startTime > 100) return false;// Nếu tín hiệu LOW vượt quá 100us thì lỗi 
  }
   // Sau đó là tín hiệu HIGH khoảng 80us
  startTime = micros();
  while (digitalRead(DHTPIN) == HIGH) {
    if (micros() - startTime > 100) return false;// Nếu tín hiệu HIGH vượt quá 100us thì lỗi 
  }

  // Đọc lần lượt 5 byte dữ liệu gửi lần lượt 
  RH_high    = Readbit();  // Byte 1: phần nguyên độ ẩm
  RH_low     = Readbit();  // Byte 2: phần thập phân độ ẩm (thường là 0)
  temp_high  = Readbit();  // Byte 3: phần nguyên nhiệt độ
  temp_low   = Readbit();  // Byte 4: phần thập phân nhiệt độ
  checksum   = Readbit();  // Byte 5: checksum dùng để kiểm tra lỗi

  // Tính checksum để kiểm tra dữ liệu truyền có đúng không 
  uint8_t check = RH_high + RH_low + temp_high + temp_low;
  return uint8_t(checksum) == check;// so sánh với byte dữ liệu 5 
}

// Khởi tạo lớp DHT để đóng gói thao tác đọc DHT22
class DHT {
public:
  void begin() {                     // Setup để đo 
    Serial.begin(115200);           // Khởi động Serial để in giá trị
    pinMode(DHTPIN, OUTPUT);        // Thiết lập chân DATA là OUTPUT
    pinMode(LEDWAR, OUTPUT);
    pinMode(COIWAR, OUTPUT);
    digitalWrite(DHTPIN, HIGH);  // Đặt chân DATA lên HIGH (idle)
    //setup_lcd();   
    //guiSMS.begin();
    delay(2000);                    // Chờ cảm biến ổn định
  }

  // Đọc nhiệt độ từ DHT22
  float runTemp() {
    if (readDHT22()) {
      // Ghép 2 byte thành giá trị 16-bit, rồi chia cho 10 để có đơn vị độ C
      temp = ((temp_high << 8) | temp_low) / 10.0;
     // Serial.print("Nhiet do: "); Serial.print(temp); Serial.println(" *C");
        hum = ((RH_high << 8) | RH_low) / 10.0;
      //Serial.print("Do am: "); Serial.print(hum); Serial.println(" %");
    //   lcd.setCursor(0, 0);
    // lcd.print("Nhiet do: ");
    // lcd.print(temp);
    // lcd.print(" C");

    // lcd.setCursor(0, 1);
    // lcd.print("Do am: ");
    // lcd.print(hum);
    // lcd.print(" %");
    } else {
    //   lcd.setCursor(0, 0);
    // lcd.print("Doc DHT that bai!");
      Serial.println("Loi doc DHT22!");
    }
    
    if ( temp >=25) {
      digitalWrite(LEDWAR, HIGH);
      digitalWrite(COIWAR, HIGH);
     // guiSMS.run();
    }
   else {
    digitalWrite(LEDWAR, LOW);
    digitalWrite(COIWAR, LOW);
   }
    return temp;
  }

  // Đọc độ ẩm từ DHT22
  float runHum() {
    delay(2000);  // Delay giữa 2 lần đọc
    return hum;
  }
} DHT22;  // Tạo một đối tượng toàn cục tên là DHT22
