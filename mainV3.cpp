/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include <cstdio>
#define Tijd 1111
#define Tijd_remmen 500
#define snelheid_1 0.50
#define snelheid_2 1 
#define snelheid_draaien snelheid_1/2
#define snelheid_draaien_1 snelheid_1/3
#define Snelheid_afwijking snelheid_1-0.01
#define Snelheid_afwijking_snel snelheid_2
#define waardesensor 86


    DigitalIn IR_sensor_OL(D14);
    DigitalIn IR_sensor_OR(D15);

    DigitalOut LM_A(D2);
    DigitalOut LM_V(D4);
    DigitalOut RM_A(D5);
    DigitalOut RM_V(D7);
    DigitalOut Led_Blauw(D13);
    DigitalOut Led_Groen(D12);

    AnalogIn IR_sensor_VL(A1);
    AnalogIn IR_sensor_VLM(A2);
    AnalogIn IR_sensor_VRM(A3);
    AnalogIn IR_sensor_VR(A4);
    AnalogIn IR_sensor_A(A0);

    PwmOut Enable_L(D3);
    PwmOut Enable_R(D6);

    

    InterruptIn Knop_Rood(D9);
    InterruptIn Knop_Blauw(D10);
    InterruptIn Knop_Groen(D8);

    bool flag_Knop_Rood_ingedrukt = false;    
    bool flag_Knop_Blauw_ingedrukt = false;
    bool flag_Knop_Groen_ingedrukt = false;
    bool Snel = false;
    int counter = 0;
    int counter1 = 0;
    Timer debounce;
    int huidigstate = 0;
    Timer timer;

    

void Knop_rood(){
    flag_Knop_Rood_ingedrukt = true;
    flag_Knop_Groen_ingedrukt = false;
    LM_V = false;
    LM_A = false;
    RM_V = false;
    RM_A = false;
    Led_Groen = 0;
    counter1 = 0;
    Enable_L.write(0);
    Enable_R.write(0);
    timer.reset();

}
void Knop_blauw(){
    debounce.start();
    if(debounce.read_ms() >= 200 && counter == 0 && !flag_Knop_Groen_ingedrukt){
        counter++;
        Led_Blauw = 1;
        Snel = true; 
        debounce.reset();
                    
    }
    else if(debounce.read_ms() >= 200 && counter == 1 && !flag_Knop_Groen_ingedrukt){
        counter--;
        Led_Blauw = 0;
        Snel = false; 
        debounce.reset();
                    
    }
}
void Knop_groen(){
    flag_Knop_Groen_ingedrukt = true;
}

enum State {
    Rust,
    Rijden,
    Rechtachter_rijden,
    Linksachter_rijden,
    Naarvoren_rijden,
    Rechts_draaien,
    Links_draaien,
};

int main(){
    State state = State::Rust;
    bool entry = true;
    Timer timer1;
    counter = 0;


    while(true) {
        Knop_Rood.rise(Knop_rood);
        Knop_Groen.rise(Knop_groen);
        Knop_Blauw.rise(Knop_blauw);

        float waarde_A = IR_sensor_A.read()*100;
        float waarde_VL = IR_sensor_VL.read()*100;
        float waarde_VLM = IR_sensor_VLM.read()*100;
        float waarde_VR = IR_sensor_VR.read()*100;
        float waarde_VRM = IR_sensor_VRM.read()*100;

      /*  printf("waarde_a %f\r\n", waarde_A);
        printf("waarde_vr %f\r\n", waarde_VR);
        printf("waarde_vrm %f\r\n", waarde_VRM);
        printf("waarde_vl %f\r\n", waarde_VL);
        printf("waarde_vlm %f\r\n", waarde_VLM);

        ThisThread::sleep_for(1500ms);
    */


        switch(state) {
        case Rust:
            if(entry) {
                entry = false;
                flag_Knop_Rood_ingedrukt = false;
                huidigstate = 1;
            }
                           

            if(flag_Knop_Groen_ingedrukt == true) {
                entry = true;
                state = State::Rijden;
            }
            break;

        case Rijden:
            if(entry) {
                entry = false;
                Led_Groen = 1;
                LM_V = true;
                LM_A = false;
                RM_V = true;
                RM_A = false;
                huidigstate = 2;
            }
            if(Snel == true){
                Enable_L.write(Snelheid_afwijking_snel);
                Enable_R.write(snelheid_2);
               huidigstate = 10;
            }
            else{
                Enable_L.write(snelheid_1);
                Enable_R.write(Snelheid_afwijking);
                
            }


            if(waarde_VL <= waardesensor) {
                entry = true;
                state = State::Rechts_draaien;
                
            }

            if(waarde_VR <= waardesensor) {
                entry = true;
                state = State::Links_draaien;
            
            }

            if(waarde_VLM <= waardesensor) {
                entry = true;
                state = State::Rechtachter_rijden;
            }

            if(waarde_VRM <= waardesensor) {
                entry = true;
                state = State::Linksachter_rijden;
            }

            if(waarde_A <= waardesensor) {
                entry = true;
                state = State::Naarvoren_rijden;
            }

            if(IR_sensor_OR == 1) {
                state = State::Linksachter_rijden;
                entry = true;
            }

            if(IR_sensor_OL == 1) {
                state = State::Rechtachter_rijden;
                entry = true;
            }
            if(flag_Knop_Rood_ingedrukt == true) {
                entry = true;
                state = State::Rust;
            }

            break;

        case Rechtachter_rijden:
            if(entry) {
                entry = false;
                LM_V = false;
                LM_A = false;
                RM_V = false;
                RM_A = false;
                Enable_L.write(0);
                Enable_R.write(0);
                huidigstate = 3;
                counter1++;
                if(counter1 == 1){
                    Enable_L.write(snelheid_draaien);
                    Enable_R.write(snelheid_draaien_1);
                    LM_V = false;
                    LM_A = true;
                    RM_V = false;
                    RM_A = true;
                    timer.start();
                }
                
            }

            if(timer.read_ms() > Tijd) {
                state = State::Rijden;
                entry = true;
                timer.reset();
                counter1--;
            }
            if(flag_Knop_Rood_ingedrukt == true) {
                entry = true;
                state = State::Rust;
            }

            break;

        case Linksachter_rijden:
            if(entry) {
                entry = false;
                LM_V = false;
                LM_A = false;
                RM_V = false;
                RM_A = false;
                Enable_L.write(0);
                Enable_R.write(0);
                counter1++;
                huidigstate = 4;
                if(counter1 == 1){
                    Enable_L.write(snelheid_draaien_1);
                    Enable_R.write(snelheid_draaien);
                    LM_V = false;
                    LM_A = true;
                    RM_V = false;
                    RM_A = true;
                    timer.start();
                }
            }
           

    
            if(timer.read_ms() > Tijd) {
                state = State::Rijden;
                entry = true;
                timer.reset();
                counter1--;
            }
            if(flag_Knop_Rood_ingedrukt == true) {
                entry = true;
                state = State::Rust;
            }

            break;

        case Naarvoren_rijden:
            if(entry) {
                entry = false;
                LM_V = false;
                LM_A = false;
                RM_V = false;
                RM_A = false;
                Enable_L.write(0);
                Enable_R.write(0);
                counter1++;
                huidigstate = 5;
                if(counter1 == 1){
                    Enable_L.write(snelheid_1);
                    Enable_R.write(snelheid_1);
                    LM_V = true;
                    LM_A = false;
                    RM_V = true;
                    RM_A = false;
                    timer.start();
                    timer1.reset();
                }
            }

            if(timer.read_ms() > Tijd) {
                state = State::Rechts_draaien;
                entry = true;
                timer.reset();
                counter1--;
            }
            if(flag_Knop_Rood_ingedrukt == true) {
                entry = true;
                state = State::Rust;
            }

            break;

        case Rechts_draaien:
            if(entry) {
                entry = false;
                LM_V = false;
                LM_A = false;
                RM_V = false;
                RM_A = false;
                Enable_L.write(0);
                Enable_R.write(0);
                counter1++;
                huidigstate = 6;
                if(counter1 == 1){
                    Enable_R.write(snelheid_draaien);
                    LM_V = false;
                    LM_A = false;
                    RM_V = false;
                    RM_A = true;
                    timer.start();
                }
            }

            if(timer.read_ms() > Tijd) {
                state = State::Rijden;
                entry = true;
                timer.reset();
                counter1--;
            }
            if(flag_Knop_Rood_ingedrukt == true) {
                entry = true;
                state = State::Rust;
            }

            break;

        case Links_draaien:
            if(entry) {
                entry = false;
                LM_V = false;
                LM_A = false;
                RM_V = false;
                RM_A = false;
                Enable_L.write(0);
                Enable_R.write(0);
                counter1++;
                huidigstate = 7;
                if(counter1 == 1){
                    Enable_L.write(snelheid_draaien);
                    LM_V = false;
                    LM_A = true;
                    RM_V = false;
                    RM_A = false;
                    timer.start();
                }
            }
            
            

            if(timer.read_ms() > Tijd) {
                state = State::Rijden;
                entry = true;
                timer.reset();
                counter1--;
            }
            if(flag_Knop_Rood_ingedrukt == true) {
                entry = true;
                state = State::Rust;
            }

            break;

        default:
            break;


        
        }
    timer.start();
    if (timer.read_ms() >= 5000){
        printf("state %d\r\n", huidigstate);
        printf("waarde_a %f\r\n", waarde_A);
        printf("waarde_vr %f\r\n", waarde_VR);
        printf("waarde_vrm %f\r\n", waarde_VRM);
        printf("waarde_vl %f\r\n", waarde_VL);
        printf("waarde_vlm %f\r\n", waarde_VLM);
        timer.reset();
    }
    

    }

}










