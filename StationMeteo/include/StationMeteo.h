#pragma once
#include <Arduino.h>

class StationMeteo
{
    public:
        StationMeteo();
        void afficherLCD(); 
        void reconnect() ;
        void executerMqtt();
        void afficherDels();
        void reset();
        void afficherDelHumidite();
        
    private: 
        void afficherRvb(long p_Coleur);
};