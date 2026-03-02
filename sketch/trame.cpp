#include "trame.h"

// initialisations des variables statics
char Trame::valid = '\0';
uint8_t Trame::fixQuality = 0;
char Trame::_bufRMC[12][CHAMP_MAX_LEN];
char Trame::_bufGGA[14][CHAMP_MAX_LEN];
char Trame::_bufGSA[17][CHAMP_MAX_LEN];

bool Trame::estValide() const {
  return (this->valid == 'A' && this->fixQuality > 0);
  //ou : return (trame::valid == 'A' && trame::fixQuality > 0);
}

// Initialisation classe Trame
Trame::Trame(char* uneTrame)
  :  _uneTrame(uneTrame),
     _type(Type::INCONNU)
{
  // initialisation du tableau de sat IDs
  for (uint8_t i = 0; i < 12; ++i) {
    satID[i] = 0;
  }
}

Trame::Type Trame::detectType()
{
  if      (!strncmp(_uneTrame, "$GPRMC", 6)) _type = Type::GPRMC;
  else if (!strncmp(_uneTrame, "$GPGGA", 6)) _type = Type::GPGGA;
  else if (!strncmp(_uneTrame, "$GPGSA", 6)) _type = Type::GPGSA;
  else                                       _type = Type::INCONNU;
  return _type;
}


bool Trame::extrait()
{
  if (!_uneTrame || !_uneTrame[0])     return false;
  if (detectType() == Type::INCONNU)   return false;

  /* 1) on choisit le buffer adapté */
  char (*champ)[CHAMP_MAX_LEN] = nullptr;
  uint8_t nbChamps = static_cast<uint8_t>(_type);   // 12 / 14 / 17

  switch (_type) {
    case Type::GPRMC: champ = _bufRMC; break;
    case Type::GPGGA: champ = _bufGGA; break;
    case Type::GPGSA: champ = _bufGSA; break;
    default:          return false;
  }
  
  /* 2) découpage de la phrase dans champ[][] */
  char *p = _uneTrame; //pointeur *p sur l'adresse de début de la chaine AZT _uneTrame
  // sauter jusqu’à la première virgule
  while (*p && *p != ',') p++;
  if (*p != ',') return false; // pas de virgule dans toute la trame
  else if (*p == ',') p++; // ici on se trouve à la première virgule de la trame

  uint8_t idxChamp = 0, idxCar = 0;
  while (*p && idxChamp < nbChamps) {
    if (*p == ',' || *p == '*') {
      champ[idxChamp++][idxCar] = '\0'; idxCar = 0;
      if (*p == '*') break;          // on s’arrête au checksum
    }
    else if (idxCar < CHAMP_MAX_LEN - 1) champ[idxChamp][idxCar++] = *p;
    p++;
  }
  /* si la boucle s’est arrêtée sur '\0', il faut aussi terminer le champ */
  if (idxCar < CHAMP_MAX_LEN) champ[idxChamp][idxCar] = '\0';
  if (idxChamp < nbChamps - 1) return false;

  /* 3) dispatch vers le parseur correspondant */
  switch (_type) {
    case Type::GPRMC: return parseRMC(champ);
    case Type::GPGGA: return parseGGA(champ);
    case Type::GPGSA: return parseGSA(champ);
    default:          return false;
  }
}

bool Trame::parseRMC(char champ[][CHAMP_MAX_LEN]) {
  //le champ de l’heure doit faire au moins 6 caractères
  if (mon_strlen(champ[0]) < 6) return false;

  //extraction de date/heure
  heure   = (champ[0][0] - '0') * 10 + (champ[0][1] - '0');
  minute  = (champ[0][2] - '0') * 10 + (champ[0][3] - '0');
  seconde = (champ[0][4] - '0') * 10 + (champ[0][5] - '0');

  //données valides
  valid = champ[1][0];
  if (valid != 'A') return false;

  // Latitude
  if (mon_strlen(champ[2]) < 4) return false;
  int lat_deg = monAtoi(champ[2]) / 100;
  latitude = lat_deg + atof(champ[2] + 2) / 60.0;
  latitude_direction = champ[3][0];

  // Longitude
  if (mon_strlen(champ[4]) < 5) return false;
  int lon_deg = monAtoi(champ[4]) / 100;
  longitude = lon_deg + atof(champ[4] + 2) / 60.0;
  longitude_direction = champ[5][0];

  // vitesse
  vitesse = atof(champ[6]) * 1.852; // vitesse en km/h

  // route
  route = atof(champ[7]);

  // Date
  if (mon_strlen(champ[8]) < 6) return false;
  jour   = (champ[8][0] - '0') * 10 + (champ[8][1] - '0');
  mois   = (champ[8][2] - '0') * 10 + (champ[8][3] - '0');
  annee  = (champ[8][4] - '0') * 10 + (champ[8][5] - '0');

  return true;
}

bool Trame::parseGGA(char champ[][CHAMP_MAX_LEN]) {

  // quality & sats
  fixQuality = monAtoi(champ[5]);
  if (fixQuality == 0) return false;  // pas de fix
  numSat     = monAtoi(champ[6]);
  hdop       = atof(champ[7]);

  // altitude / geoid
  altitude        = atof(champ[8]);
  altitudeUnit    = champ[9][0];
  geoidSeparation = atof(champ[10]);
  geoidUnit       = champ[11][0];

  return true;
}

bool Trame::parseGSA(char champ[][CHAMP_MAX_LEN]) {
  selMode = champ[0][0];
  fixType = champ[1][0] - '0';
  // 12 sat IDs
  for (int i = 0; i < 12; ++i) {
    satID[i] = (mon_strlen(champ[2 + i]) > 0) ? monAtoi(champ[2 + i]) : 0;
  }
  pdop     = atof(champ[14]);
  hdop_gsa = atof(champ[15]);
  vdop     = atof(champ[16]);
  return true;
}

uint8_t Trame::mon_strlen(const char *cstring) { // ici pas de size_t (unsigned int) car len est toujours compris entre 0 et 255
  uint8_t len = 0;
  while (cstring[len] != '\0') {
    len++;
  }
  return len;
}


// je suis sur 328p : la limite est entre -32768 et 32767,
//d'autre part la validité des latitudes et longitudes est assurée (valid)
int Trame::monAtoi(const char *s)
{
  // 1. Ignorer les espaces
  while (*s == ' ' || *s == '\t') ++s;

  // 2. Signe éventuel
  bool negatif = false;
  if (*s == '+' || *s == '-') {
    negatif = (*s == '-');
    ++s;
  }

  // 3. Accumuler la valeur
  int32_t val = 0;
  while (*s >= '0' && *s <= '9') {
    val = val * 10 + (*s - '0');   // ← évite la lourde lib std
    ++s;
  }

  return negatif ? -val : val;
}


// Destruction - par principe
Trame::~Trame() {}
