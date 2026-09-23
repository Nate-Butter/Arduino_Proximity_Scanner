#include <Servo.h>
#include <LiquidCrystal.h>


// OBJECTS
Servo scanner;

LiquidCrystal lcd(10, 9, 5, 4, 3, 2);



// PIN DEFINITIONS
const int servoPin = 11;

const int trigPin = 13;
const int echoPin = 12;

const int redPin = A0;
const int greenPin = A1;
const int bluePin = A2;

const int buzzerPin = A5;



// SERVO VARIABLES
int angle = 20;
int direction = 1;

unsigned long previousServoTime = 0;
const unsigned long servoInterval = 25;



// ULTRASONIC VARIABLES
unsigned long previousSensorTime = 0;
const unsigned long sensorInterval = 100;

unsigned long duration = 0;
float distanceInches = 0;



// LCD VARIABLES
unsigned long previousLCDTime = 0;
const unsigned long lcdInterval = 150;



// BUZZER VARIABLES
unsigned long previousBeepTime = 0;
bool buzzerState = false;



// DISTANCE THRESHOLDS
const float warningDistance = 48.0;
const float dangerDistance = 24.0;




void setup()
{
    scanner.attach(servoPin);
    scanner.write(angle);



    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    
    
    
    
    // RGB LED
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);

   
    pinMode(buzzerPin, OUTPUT);

    


    lcd.begin(16, 2);
    lcd.setCursor(0, 0);
    lcd.print("Scanner Active");




    // Start with LED and buzzer off
    setColor(LOW, LOW, LOW);
    noTone(buzzerPin);



   
    Serial.begin(9600);
}






void loop()
{
    updateServo();


    measureDistance();

    updateLCD();



    updateWarningSystem();
}





//Servo sweeping motion
void updateServo()
{
    unsigned long currentTime = millis();

    if (currentTime - previousServoTime >= servoInterval)
    {
        previousServoTime = currentTime;

        angle += direction;

        if (angle >= 160)
        {
            angle = 160;
            direction = -1;
        }

        if (angle <= 20)
        {
            angle = 20;
            direction = 1;
        }

        scanner.write(angle);
    }
}



//Ultrasonic sensor measuring
void measureDistance()
{
    unsigned long currentTime = millis();

    if (currentTime - previousSensorTime >= sensorInterval)
    {
        previousSensorTime = currentTime;

        
        digitalWrite(trigPin, LOW);
        delayMicroseconds(2);

        digitalWrite(trigPin, HIGH);
        delayMicroseconds(10);

        digitalWrite(trigPin, LOW);

        
        duration = pulseIn(echoPin, HIGH, 25000);

        // Only update if valid
        if (duration > 0)
        {
            
          //speed of sound times how long / 2(there and back)
            distanceInches = duration * 0.0135 / 2.0;
        }

        Serial.print("Angle: ");
        Serial.print(angle);

        Serial.print("  Distance: ");
        Serial.print(distanceInches, 1);

        Serial.println(" in");
    }
}




void updateLCD()
{
    unsigned long currentTime = millis();

    if (currentTime - previousLCDTime >= lcdInterval)
    {
        previousLCDTime = currentTime;

        int feet = distanceInches / 12;

        float remainingInches =
            distanceInches - (feet * 12);

        lcd.setCursor(0, 0);
        lcd.print("Distance:       ");

        lcd.setCursor(0, 1);

        lcd.print(feet);
        lcd.print(" ft ");

        lcd.print(remainingInches, 1);
        lcd.print(" in   ");
    }
}



//LED and Buzzer control system
void updateWarningSystem()
{
    //bad reading
    if (distanceInches <= 0)
    {
        setColor(LOW, LOW, LOW);

        noTone(buzzerPin);
        buzzerState = false;

        return;
    }

    // SAFE
    if (distanceInches > warningDistance)
    {
        
        setColor(LOW, HIGH, LOW);

        noTone(buzzerPin);
        buzzerState = false;
    }

    // WARNING
    else if (distanceInches > dangerDistance)
    {
        // Yellow = red + green
        setColor(HIGH, HIGH, LOW);

        updateBuzzer(
            700,   
            800    
        );
    }

    // DANGER
    else
    {
        
        setColor(HIGH, LOW, LOW);

        updateBuzzer(
            150,    
            1500    
        );
    }
}
//buzzer function for warning system
void updateBuzzer(
    unsigned long beepInterval,
    int frequency
)
{
    unsigned long currentTime = millis();

    if (currentTime - previousBeepTime >= beepInterval)
    {
        previousBeepTime = currentTime;

        buzzerState = !buzzerState;

        if (buzzerState)
        {
            tone(buzzerPin, frequency);
        }
        else
        {
            noTone(buzzerPin);


        }

    }


}




// RGB LED FUNCTION+ might need to switch to 3 seperate led for hardware issue


void setColor(bool red, bool green, bool blue)
{
    digitalWrite(redPin, red);
    digitalWrite(greenPin, green);
    digitalWrite(bluePin, blue);
}