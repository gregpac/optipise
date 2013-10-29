struct 
{ 
  int photocellReading;
} lum ;

void lum_setup()
{
};

void lum_loop()
{
  int photocellPin = 0; // the cell and 10K pulldown are connected to a0
  lum.photocellReading = analogRead(photocellPin);
};
