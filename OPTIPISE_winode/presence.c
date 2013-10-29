volatile int validPres; 

void validPresFonction()
{
  validPres = 0;
}

void presence_setup()
{
  validPres = 1;
  attachInterrupt(1, validPresFonction, 0);
};






