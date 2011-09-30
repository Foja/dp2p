
#include "utility.h"
#include "PEER/peer.h"
int main()
{
  struct in_addr ip;
  /*    char *dir_down="PEER/cartella_condivisa/";//cartella dove l'uploader tiene i file disponibili da condividere
      char* parziale;
      char buffname[255];
      char* app="pavone.JPG";
      int i;
      for(i=0;i<255;i++)
      {
        if(i<strlen(app))
          buffname[i]=*(app+i);
        else
          buffname[i]='\0';	
      }

      printf("buffname %s \n",app);
      parziale=search_path_file(dir_down,buffname,1);

     	printf( "percorso file:%s\n", parziale );
      //printf("search:%d\n", search_file(dir_down,app,1));
*/
//-------------------------------------------------
  uploadAnd (); //restituisce 1 se il download va a buon fine, 0 altrimenti
  return 0;

}


