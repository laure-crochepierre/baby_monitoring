/******************************************************************************
 * Description des branchements :
 
 * Les branchements du micro sont les suivants:
 * (Capteur sonore -> pin de branchement sur l'Arduino)
 * GND → GND
 * VCC → 5V
 * Gate → Pin 4
 * Envelope → A0
 
 * Les branchements du haut parleur sont les suivants: 
 * première borne du HP reliée à une résistance elle-même branchée sur le pin 8
 * la deuxième borne reliée à GND 
 
 * Les branchements du capteur de température LM35DZ sont les suivants : 
 * côté plat vers soi, borne de droite reliée à GND
 * celle du milieu reliée à A1
 * celle de gauche à 5V
 
 * Les branchements de l'accéléromètre sont les suivants :
 * GND -> GND
 * VCC -> 3,3V 
 * CS -> 3,3V
 * sda -> A4
 * scl -> A5
 
 * Les branchements du module bluetooth sont les suivants : 
 * GND -> GND
 * VCC -> 3,3V
 * Tx -> Pin 2
 * RX -> Pin 3
 ******************************************************************************/
#include <SoftwareSerial.h>
// utilisation de la bibliothèque Wire pour l'accelerometre
#include <Wire.h>


// Initialisation de l'accelerometre 
const byte HAUT = HIGH;
const byte BAS = LOW;
const byte NON = false;
const byte OUI = true;
const byte VRAI = true;
const byte FAUX = false;

// Broche d'arduino pour communication I2C avec l'accéléromètre ADXL345
const int ACCELEROMETRE_SCL = A5;
const int ACCELEROMETRE_SDA = A4;

// Adresses de communication I2C avec l'accéléromètre ADXL345
const char Accelerometre3AxesAdresse = 0x53;
byte Accelerometre3AxesMemoire [12];
const char POWER_CTL = 0x2D; //Registre du Power Control
const char DATA_FORMAT = 0x31;
const char DATAX0 = 0x32;  //Axe-X Donnée 0
const char DATAX1 = 0x33; //Axe-X Donnée 1
const char DATAY0 = 0x34; //Axe-Y Donnée 0
const char DATAY1 = 0x35; //Axe-Y Donnée 1
const char DATAZ0 = 0x36; //Axe-Z Donnée 0
const char DATAZ1 = 0x37; //Axe-Z Donnée 1

const char Accelerometre_Precision2G = 0x00;
const char Accelerometre_Precision4G = 0x01;
const char Accelerometre_Precision8G = 0x02;
const char Accelerometre_Precision16G = 0x03;
const char Accelerometre_ModeMesure = 0x08;

// Pour recevoir les valeurs des 3 axes de l'accéléromètre et des valeurs de l'offset.
int Accelerometre_AxeX = 0;
int Accelerometre_AxeY = 0;
int Accelerometre_AxeZ = 0;

// Définition des connections
int bluetoothTx = 2;
int bluetoothRx = 3; 

#define gate 4 // Gate relié au pin numéro 2 : à partir d'un certain seuil
// il vaut 1, quelle que soit l'intensité, et 0 sinon 
#define ground  0 // Ground 
#define analogMicro A0 // Sortie analogique connecté au pin A0 (Mesure de tension en temps réel)
#define HP 8 // Pin de sortie pour la berceuse
#define analogTemp A1
#define analogSDA A4
#define analogSCL A5

// Préparation des variables utilisées pour la berceuse
// Definition des duree des notes
#define NOIRE              1.000
#define BLANCHE           2.0*NOIRE
#define RONDE             4.0*NOIRE
#define CROCHE            NOIRE/2.0
#define DOUBLE_CROCHE     NOIRE/4.0
#define QUADRUPLE_CROCHE  NOIRE/8.0

// Definition des frequences des notes de musique
#define MUTE    0
#define DO_3  261.63
#define DO_D_3  277.18
#define RE_3  293.66
#define RE_D_3  311.13
#define MI_3  329.63
#define FA_3  349.23
#define FA_D_3  369.99
#define SOL_3 392
#define SOL_D_3 415.3
#define LA_3  440
#define LA_D_3  466.16
#define SI_3  493.88
#define DO_4  523.25
#define DO_D_4  554.37
#define RE_4  587.33
#define RE_D_4  622.25
#define MI_4  659.26
#define FA_4  698.46
#define FA_D_4  739.99
#define SOL_4 783.99
#define SOL_D_4 830.61
#define LA_4  880
#define LA_D_4  932.33
#define SI_4  987.77
#define DO_5  1046.5


//choix du pin de sortie pour la température 
int capteur = 0 ; // choix du pin de sortie
int valeur_temp = 0; // initialisation de la valeur mesurée par le capteur
int valeur_micro =0;
float temperature = 0.0; // initialisation de la température 
int etat=0;


//Definition et initialisation des messages en "global" pour permettre leur utilisation au sein de fonctions exterieures au setup() et  au loop()
  
  //en reception
  String demande_berceuse="";
  String demande_micro_bruit = "";
  String demande_temperature_statut = "";
  // en emission
  String alerteFroid="";
  String alerteChaud="";
  String alerteMvt="";
  String alerteMicro="";
  String reponseTemp="";
  String reponseMicro="";
  String reponseHP="";



// Définition du module bluetooth
SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);

// Berceuse
float melodie[]={
  500,              // durée de la noire en ms
  SI_3, BLANCHE, RE_4, NOIRE, LA_3, BLANCHE, SOL_3, CROCHE, LA_3, CROCHE,
  SI_3,  BLANCHE, RE_4, NOIRE, LA_3, BLANCHE, MUTE, NOIRE,
  SI_3, BLANCHE, RE_4, NOIRE, LA_4, BLANCHE, SOL_4, NOIRE,
  RE_4, BLANCHE, DO_4, CROCHE, SI_3, CROCHE,  LA_4, BLANCHE, MUTE, NOIRE,
  SI_3, BLANCHE, RE_4, NOIRE, LA_3, BLANCHE, SOL_3, CROCHE, LA_3, CROCHE,
  SI_3,BLANCHE, RE_4, NOIRE, LA_3, BLANCHE, MUTE, NOIRE,
  SI_3, BLANCHE, RE_4, NOIRE, LA_4, BLANCHE, SOL_4, NOIRE,
  RE_4, BLANCHE, RE_4, BLANCHE, MUTE, BLANCHE,
  RE_4, BLANCHE, DO_4, CROCHE, SI_3, CROCHE, DO_4, CROCHE, SI_3, CROCHE, SOL_3, BLANCHE,
  DO_4, BLANCHE,  SI_3, CROCHE, LA_3, CROCHE, SI_3, CROCHE, LA_3, CROCHE, MI_3, BLANCHE,
  RE_4, BLANCHE,  DO_4, CROCHE, SI_3, CROCHE, DO_4, CROCHE, SI_3, CROCHE, SOL_3, NOIRE, RE_4, NOIRE,
  SOL_4, BLANCHE, SOL_4, BLANCHE
};

// Initialisation du matériel
void setup(){
  Serial.begin(9600);

  // définition du pin de sortie 
  pinMode(HP, OUTPUT);


  // Initialisation de la communication i2c bus pour le capteur d'acceleration.
  Wire.begin ();
  // Mettre le ADXL345 à plage +/-4G en écrivant la valeur 0x01 dans le registre DATA_FORMAT.
  AccelerometreConfigure (DATA_FORMAT, Accelerometre_Precision4G);
  // Mettre le ADXL345 en mode de mesure en écrivant 0x08 dans le registre POWER_CTL.
  AccelerometreConfigure (POWER_CTL, Accelerometre_ModeMesure);
  
  //Initialise la connexion bluetooth avec le module
  bluetooth.begin(115200);
  bluetooth.print("$$$");
  delay(100);
  bluetooth.println("U,9600,N");
  bluetooth.begin(9600);


 // On informe de la bonne initialisation du matériel
  Serial.println("Materiel initialise");
    
}



// Boucle répétée 
void loop()
{ 
  //Lit ce qui a été envoyé par bluetooth et le stocke dans la variable de type String sent.
  String sent;
  if(bluetooth.available()){
    sent = (String)bluetooth.readString();
    }


 // test de la température via le capteur analogique LM35DZ
  valeur_temp = analogRead(analogTemp); // Récupération de la mesure faite par le capteur
  temperature = valeur_temp/ 2.056; // Définition de la température à partir de la mesure de tension

  //test du micro
  valeur_micro = analogRead(analogMicro);
  //Serial.println(valeur_micro);
  etat = etatBebe(valeur_micro);
  
  //test de l'accelerometre
    // On teste ici l'arrêt de mouvement pendant 6 secondes 
  AccelerometreLecture (); // on récupère les données de l'accéléromètre dans des buffers 
  const int X0 = Accelerometre_AxeX; // valeur selon l'axe des x à t0
  const int Y0 = Accelerometre_AxeY; // valeur selon l'axe des y à t0
  const int Z0 = Accelerometre_AxeZ; // valeur selon l'axe des z à t0
  delay (1000);

  AccelerometreLecture ();
  const int X1 = Accelerometre_AxeX; // valeur selon l'axe des x à t1
  const int Y1 = Accelerometre_AxeY; // valeur selon l'axe des y à t1
  const int Z1 = Accelerometre_AxeZ; // valeur selon l'axe des z à t1
  
  //On crée des booléens qui comparent les valeurs des axes à t0 et t1. Si l'accéléromètre détecte un mouvement, le booléen vaut 0, sinon, il vaut 1.
  bool Xa = (X0 == X1) || (X0 == (X1 + 1)) || (X0 == (X1 - 1)); 
  bool Ya = (Y0 == Y1) || (Y0 == (Y1 + 1)) || (Y0 == (Y1 - 1));
  bool Za = (Z0 == Z1) || (Z0 == (Z1 + 1)) || (Z0 == (Z1 - 1));
  bool R1 = Xa && Ya && Za; // R1 vaut 0 si on a un mouvement selon au moins un des axes; sinon il vaut 1
  delay (1000); // On a une pause de 1 seconde

  AccelerometreLecture ();
  const int X2 = Accelerometre_AxeX; // valeur selon l'axe des x à t2
  const int Y2 = Accelerometre_AxeY; // valeur selon l'axe des y à t2
  const int Z2 = Accelerometre_AxeZ; // valeur selon l'axe des z à t2
  bool R2 = test (R1, X1, Y1, Z1, X2, Y2, Z2); // R2 vaut 0 si on a un mouvement selon au moins un des axes, entre t0 et t2; sinon il vaut 1
  delay (1000);

  AccelerometreLecture ();
  const int X3 = Accelerometre_AxeX; // valeur selon l'axe des x à t3
  const int Y3 = Accelerometre_AxeY; // valeur selon l'axe des y à t3
  const int Z3 = Accelerometre_AxeZ; // valeur selon l'axe des z à t3
  bool R3 = test (R2 ,X2 , Y2, Z2, X3, Y3, Z3); // R3 vaut 0 si on a un mouvement selon au moins un des axes, entre t0 et t3; sinon il vaut 1
  delay (1000);

  AccelerometreLecture ();
  const int X4 = Accelerometre_AxeX; // valeur selon l'axe des x à t4
  const int Y4 = Accelerometre_AxeY; // valeur selon l'axe des y à t4
  const int Z4 = Accelerometre_AxeZ; // valeur selon l'axe des z à t4
  bool R4 = test (R3, X3, Y3, Z3, X4, Y4, Z4); // R4 vaut 0 si on a un mouvement selon au moins un des axes, entre t0 et t4; sinon il vaut 1
  delay (1000);

  AccelerometreLecture ();
  const int X5 = Accelerometre_AxeX; // valeur selon l'axe des x à t5
  const int Y5 = Accelerometre_AxeY; // valeur selon l'axe des y à t5
  const int Z5 = Accelerometre_AxeZ; // valeur selon l'axe des z à t5
  bool R5 = test (R4, X4, Y4, Z4, X5, Y5, Z5) ; // R5 vaut 0 si on a un mouvement selon au moins un des axes, entre t0 et t3; sinon il vaut 1
  delay (1000);

  AccelerometreLecture ();
  const int X6 = Accelerometre_AxeX; // valeur selon l'axe des x à t6
  const int Y6 = Accelerometre_AxeY; // valeur selon l'axe des y à t6
  const int Z6 = Accelerometre_AxeZ; // valeur selon l'axe des z à t6
  bool R6 = test (R5 , X5, Y5, Z5, X6, Y6, Z6) ; // R6 vaut 0 si on a un mouvement selon au moins un des axes, entre t0 et t3; sinon il vaut 1


  if ( R6 == 1 ) { // on n'a pas détecté de mouvement, selon aucun axe, depuis 5 secondes
    bluetooth.print(alerteMvt);
  }
  delay (1000);
  //fin du test de l'accelerometre
  
  
  //Mise à jour des messages
  //en reception
  demande_berceuse="#demande:hp+berceuse/0~";
  demande_micro_bruit = "#demande:micro+bruit/0~";
  demande_temperature_statut = "#demande:temperature+statut/0~";
      
    
   // en emission -!-apres-!- mise a jour des variables temperatures,etat
   alerteFroid= "#alerte:temperature+froid/" + (String)temperature + "~";
   alerteChaud= "#alerte:temperature+chaud/" + (String)temperature + "~";
   alerteMvt= "#alerte:mouvement+immobile/0~";
   alerteMicro = "#alerte:micro+bruit/" + (String) valeur_micro +"~";
   reponseTemp= "#reponse:temperature+statut/" + (String) temperature + "~";
   reponseMicro = "#reponse:micro+bruit/" + (String) etat + "~";
   reponseHP= "#reponse:hp+berceuse/1~";
   
  /***********************ALERTES****************************/
  
  if (etat==3)
  {
    
    //On envoie le message d'alerte
    bluetooth.print(alerteMicro); 
    delay(100);//sinon fait planter l'appli
    
  }
         
        
 // Température
//Serial.println(temperature); // Affichage de la température non stop..
 
 //Si la temperature est jugée trop basse, on envoie un message de type alerte
if (temperature<= 15 ){
   
    //On envoie le message d'alerte
    bluetooth.print(alerteFroid); 
    delay(100);//sinon fait planter l'appli
    
   
 }
 
  //Si la temperature est jugée trop haute, on envoie un message de type alerte
  if (temperature >= 25){//sinon 21
    
    //On envoie le message d'alerte
    bluetooth.print(alerteChaud); 
    delay(100);//sinon fait planter l'appli
   
    
  }


//Une fois toutes les valeurs actualisées, on analyse l'éventuelle trame bluetooth receptionnée en envoyant son contenu à la fonction analyse_message, définie en dehors de la loop.
 analyse_message(sent);

  
}//END LOOP


  //Fonction qui lance la berceuse et envoie une trame bluetooth informant que la berceuse a bien été lancée
  void berceuse(){    
    
 
  bluetooth.print(reponseHP);  //envoi du message avant de lancer, sinon la mère devrait attendre la fin de la berceuse pour savoir si elle bien été lancée...
  
  int nombreNote = sizeof(melodie) / (2*sizeof(float));
  int indexDerniereNote = nombreNote*2 - 1;
  int dureeNoire = melodie[0];

  for( int indexNote=1; indexNote<=indexDerniereNote; indexNote+=2){
    if(melodie[indexNote] != MUTE){
      tone(HP, melodie[indexNote]);
    }
   delay(melodie[indexNote + 1] * dureeNoire);
   noTone(HP);
   }
   
}

//Fonction permettant d'alléger le code de la loop(): elle envoie la réponse à l'application android en fonction de la trame reçue
void analyse_message(String sent)
{
  //On stocke la trame dans une nouvelle variable message au cas où...
   String message = sent;
   Serial.print(message); //On l'affiche dans le moniteur série

 //S'il s'agit d'une demande de berceuse
  if(message.equals(demande_berceuse)){
    berceuse();
    }
    
 //S'il s'agit d'une demande concernant "l'état sonore" de bébé 
  if(message.equals(demande_micro_bruit)){
    //On envoie le message de reponse
  
    bluetooth.print(reponseMicro); 
   
}
    
//S'il s'agit d'une demande de température de la pièce   
  if(message.equals(demande_temperature_statut)){
 //On envoie le message d'alerte

 bluetooth.print(reponseTemp); //test
 
    }
    
}
 

 // On teste ce que mesure le micro au travers de l'enveloppe en analogique (amplitude du signal de sortie)
char etatBebe ( int valeur ) {
  // On choisit de convertir la valeur obtenue en un entier
  int etat; 
  char statut;
  Serial.println(valeur);
  
  if(valeur <= 60){
    etat = 1;
  //  Serial.println( "Statut : Bebe dort.");
}
  
  else if( (valeur > 60) && ( valeur <= 120) ){
    etat = 2;
    //Serial.println( "Statut : Bebe est reveille.");
  }
    
  else if(valeur > 120){ // au départ on avait choisi une valeur de 30 de seuil entre bébé réveillé et bébé pleure 
    etat = 3;
    //Serial.println( "Statut : Bebe pleure.");
  }
  
  
  return etat;}  

void AccelerometreConfigure (byte address, byte val) {

  // Commencer la transmission à trois axes accéléromètre
  Wire.beginTransmission (Accelerometre3AxesAdresse);
  // Envoyer l'adresse de registre
  Wire.write (address);
  // Envoyer la valeur à écrire.
  Wire.write (val);
  // Fin de la transmission.
  Wire.endTransmission ();
}

void AccelerometreLecture () {

  uint8_t NombreOctets_a_Lire = 6;
  // Lire les données d'accélération à partir du module ADXL345.
  AccelerometreLectureMemoire (DATAX0, NombreOctets_a_Lire,
                               Accelerometre3AxesMemoire);
  // Chaque lecture de l'axe vient dans une résolution de 10 bits, soit 2 octets.
  // Première Octet significatif !
  // Donc nous convertissons les deux octets pour un « int ».
  Accelerometre_AxeX = (((int)Accelerometre3AxesMemoire[1]) << 8) | Accelerometre3AxesMemoire[0];
  Accelerometre_AxeY = (((int)Accelerometre3AxesMemoire[3]) << 8) | Accelerometre3AxesMemoire[2];
  Accelerometre_AxeZ = (((int)Accelerometre3AxesMemoire[5]) << 8) | Accelerometre3AxesMemoire[4];
  
}


void AccelerometreLectureMemoire (byte address, int num, byte Accelerometre3AxesMemoire[])
{
  // Démarrer la transmission à accéléromètre.
  Wire.beginTransmission (Accelerometre3AxesAdresse);
  // Envoie l'adresse à lire.
  Wire.write (address);
  // Fin de la transmission.
  Wire.endTransmission ();
  // Démarrer la transmission à accéléromètre.
  Wire.beginTransmission (Accelerometre3AxesAdresse);
  // Demande 6 octets à l'accéléromètre.
  Wire.requestFrom (Accelerometre3AxesAdresse, num);
  int i = 0;
  // L'accéléromètre peut envoyer moins que demandé, dans ce cas
  while (Wire.available())
  {
    // Recevoir un octet.
    Accelerometre3AxesMemoire[i] = Wire.read ();
    i++;
  }
  // Fin de la transmission.
  Wire.endTransmission ();
}


// Fonction qui permet de réaliser le test : le bébé a-t-il bougé ou non? 
bool test (bool R, bool Xp, bool Yp, bool Zp, bool X, bool Y, bool Z){
  
  bool Xb = (X == Xp) || (X == (Xp + 1)) || (X == (Xp - 1));
  bool Yb = (Y == Yp) || (Y == (Yp + 1)) || (Y == (Yp - 1));
  bool Zb = (Z == Zp) || (Z == (Zp + 1)) || (Z == (Zp - 1));
 
  bool Rbis = Xb && Yb && Zb; // Si Rbis vaut 1, l'accéléromètre n'a détecté aucun mouvement, sur aucun axe.
  return R && Rbis ; // On compare R et Rbis
}
  


