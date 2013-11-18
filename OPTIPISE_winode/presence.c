volatile int validPres; 

void validPresFonction()
{
  validPres = 1;
}

void presence_setup()
{
  validPres = 0;
  attachInterrupt(1, validPresFonction, 0);
};






