
#ifndef TRAME_H
#define TRAME_H
#include <Arduino.h>

static constexpr uint8_t CHAMP_MAX_LEN = 20;

class Trame
{
  public:
    enum class Type : uint8_t { GPRMC = 12, GPGGA = 14, GPGSA = 17, INCONNU = 0 };
    // Constructeur
    explicit Trame(char* uneTrame);

    void setSentence(char* nmea) {
      _uneTrame = nmea;
    }
    
    bool   extrait();                // décode la phrase courante
    bool   estValide() const;
    Type   type()   const {
      return _type;
    }

    static constexpr uint8_t UTC_OFFSET = 1;

    /* --- champs publics (inchangés) initialisés */
    uint8_t jour{}, mois{}, annee{}, heure{}, minute{}, seconde{};
    double  latitude{}, longitude{}, vitesse{}, route{};
    char    latitude_direction{}, longitude_direction{};
    static  char    valid;

    static  uint8_t fixQuality;
    uint8_t numSat{};  double hdop{}, altitude{}, geoidSeparation{};
    char    altitudeUnit{}, geoidUnit{};
    char    selMode{}; uint8_t fixType{}, satID[12]{};
    double  pdop{}, hdop_gsa{}, vdop{};

    // Destructeur
    ~Trame();

  private:
    char* _uneTrame;

    Type  _type { Type::INCONNU };
    
    /* buffers dédiés à chaque type */
    static char _bufRMC[12][CHAMP_MAX_LEN];
    static char _bufGGA[14][CHAMP_MAX_LEN];
    static char _bufGSA[17][CHAMP_MAX_LEN];
    
    Type detectType();
    
    /* parseurs */
    bool parseRMC(char c[][CHAMP_MAX_LEN]);
    bool parseGGA(char c[][CHAMP_MAX_LEN]);
    bool parseGSA(char c[][CHAMP_MAX_LEN]);

    uint8_t mon_strlen(const char*);
    int     monAtoi(const char*);


};
#endif
