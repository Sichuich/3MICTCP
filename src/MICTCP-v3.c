#include <mictcp.h>
#include <api/mictcp_core.h>
#define TAILLE 15

mic_tcp_sock sock;
mic_tcp_sock_addr sock_addr;
/*Variables pour le STOP & WAIT*/
int PA = 0;
int PE = 0;
int perte_tolere = 10 ;
int tab[TAILLE] = {[0 ... TAILLE-1] = 1}; //febetre glissante
int indice_prochain_mesg = 0;


/*
 * Permet de créer un socket entre l’application et MIC-TCP
 * Retourne le descripteur du socket ou bien -1 en cas d'erreur
 */
int mic_tcp_socket(start_mode sm)
{
   int result = -1;
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n"); 

   /*reglation du pourcentage des pertes sur le reseau*/
   set_loss_rate(20);  
   result = initialize_components(sm) ;
   if (result == -1 ){
    return -1 ;
   }else{
    sock.fd = 0;
    sock.state = IDLE;
    return sock.fd ;
   }
}

/*
 * Permet d’attribuer une adresse à un socket.
 * Retourne 0 si succès, et -1 en cas d’échec
 */
int mic_tcp_bind(int socket, mic_tcp_sock_addr addr)
{
  int result = -1;
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
   if (socket == sock.fd) {
     sock.addr = addr ;
     result = 0 ;
   }   
   return result;
}

/*
 * Met le socket en état d'acceptation de connexions
 * Retourne 0 si succès, -1 si erreur
 */
int mic_tcp_accept(int socket, mic_tcp_sock_addr* addr)
{
   int result = -1;
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");  
  if (socket == sock.fd) {
    sock.state = CONNECTED;
    result = 0;
  }  
  return result;
}

/*
 * Permet de réclamer l’établissement d’une connexion
 * Retourne 0 si la connexion est établie, et -1 en cas d’échec
 */
int mic_tcp_connect(int socket, mic_tcp_sock_addr addr)
{
   int result = -1;
   
  printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
  if(socket == sock.fd){
    sock.state = CONNECTED;
    sock_addr = addr;
    result = 0;
  }
  return result;
}

/*
 * Permet de réclamer l’envoi d’une donnée applicative
 * Retourne la taille des données envoyées, et -1 en cas d'erreur
 */
int mic_tcp_send (int mic_sock, char* mesg, int mesg_size)
{
  int sentsize = -1 ;
  mic_tcp_pdu PDU;
  mic_tcp_pdu ACK;

  
   printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
  
  //géneration PDU-DT contenant
  if ((mic_sock == sock.fd) && (sock.state == CONNECTED)) {   
    PDU.header.source_port = sock.addr.port;
    PDU.header.dest_port = sock_addr.port;
    PDU.header.seq_num = PE;
    PDU.header.syn = 0;
    PDU.header.ack = 0;
    PDU.header.fin = 0;

    PDU.payload.data = mesg;
    PDU.payload.size = mesg_size;
   /*Incrementation de PE*/
    PE =  ( PE + 1) % 2;

    //send PDU
    sentsize = IP_send(PDU, sock_addr);

    //Attente de l'acquitement
    sock.state = WAIT_ACK ;

    //Traitement de l'acquitement 
    ACK.payload.size = 2*sizeof(short)+2*sizeof(int)+3*sizeof(char);
    ACK.payload.data = malloc(ACK.payload.size);

    unsigned long timeout =100;
    int controle = 0 ;
    int result = -1 ;
    //on utilise la boucle puisque les pertes peuvent se produire plus d'une fois
    while(controle == 0){
        /*on a bien recu l'ack correspondant au pdu envoye*/
        if (((result=IP_recv(&(ACK),&sock_addr, timeout)) != -1) && (ACK.header.ack == 1) && (ACK.header.ack_num == PE)) { 
            controle = 1;
           /* Mise a jour de la fenetre d'analyse des pertes */
           tab[indice_prochain_mesg] = 1;
           indice_prochain_mesg = (indice_prochain_mesg + 1) % TAILLE;
      }else if (result == -1){
        /*expiration du timer avant reception d'un acquitement*/
        tab[indice_prochain_mesg] = 0;
        int  nb_mesg_recus = 0;
        /*calculs de nb des pdu bien envoyes*/
        for (int i=0; i<TAILLE; i++) {
            nb_mesg_recus += tab[i];
        } 
        if(((float)(TAILLE-nb_mesg_recus)/(float)TAILLE*100.0) <= perte_tolere){
           /*on envoie pas le PDU*/
            controle = 1;
            /*Incrementation de PE*/
            PE = ( PE + 1 ) % 2 ;
            indice_prochain_mesg = (indice_prochain_mesg + 1) % TAILLE;
        }else{
            /*cette perte n'est pas tolere , alors on renvoie le PDU*/
            sentsize =  IP_send(PDU, sock_addr);
        }
      }else {
        printf("acquitement recue differenr de celui attendu");
      }
    }
 }else{
    return -1 ; /*erreur de num_socket ou de connexion */
 }
  sock.state = CONNECTED ;
  return sentsize;

}

/*
 * Permet à l’application réceptrice de réclamer la récupération d’une donnée
 * stockée dans les buffers de réception du socket
 * Retourne le nombre d’octets lu ou bien -1 en cas d’erreur
 * NB : cette fonction fait appel à la fonction app_buffer_get()
 */
int mic_tcp_recv (int socket, char* mesg, int max_mesg_size)
{
  int deliver_size = -1;
  mic_tcp_payload Payload;
  
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");

  Payload.data = mesg;
  Payload.size = max_mesg_size;
    
  if ((socket == sock.fd) && (sock.state == CONNECTED)) {
    //WAIT for un PDU
    sock.state = WAIT_PDU ;    

    //récupérer la donnée applicative contenue dans le buffer 
    deliver_size = app_buffer_get(Payload);

    //revient state
    sock.state = CONNECTED ;
  }
  return deliver_size;  
}

/*
 * Permet de réclamer la destruction d’un socket.
 * Engendre la fermeture de la connexion suivant le modèle de TCP.
 * Retourne 0 si tout se passe bien et -1 en cas d'erreur
 */
int mic_tcp_close (int socket)
{
    printf("[MIC-TCP] Appel de la fonction :  "); printf(__FUNCTION__); printf("\n");
    return -1;
}

/*
 * Traitement d’un PDU MIC-TCP reçu (mise à jour des numéros de séquence
 * et d'acquittement, etc.) puis insère les données utiles du PDU dans
 * le buffer de réception du socket. Cette fonction utilise la fonction
 * app_buffer_put().
 */
void process_received_PDU(mic_tcp_pdu pdu, mic_tcp_sock_addr addr)
{        
    mic_tcp_pdu ACK ;
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
    // on teste le n°seq pour accepter la DU
    if (pdu.header.seq_num == PA) { 
        app_buffer_put(pdu.payload);
        
        // Incrémentation de PA 
        PA = (PA +1) % 2;
    }

    // Concstruction d'un acquitement 
    ACK.header.source_port = sock.addr.port;
    ACK.header.dest_port = addr.port;
    ACK.header.ack_num = PA;
    ACK.header.syn = 0;
    ACK.payload.size = 0;
    ACK.header.ack = 1;
    ACK.header.fin = 0;

    /* Envoi de l'acquitement */
    int s = IP_send(ACK, addr);
    if (s == -1){
        printf("IP_send erreur");
    }    
}
