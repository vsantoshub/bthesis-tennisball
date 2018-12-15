/* 
 * File:	server.h
 * Author:   Victor Santos (viic.santos@gmail.com)
 * Comment: Servidor TCP/IP do protocolo de troca de mensagens.
 *
 */

#ifndef __SERVER_H__
#define __SERVER_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    /*
       pacotes que podem ser recebidos
       */
#define SERVER_RX_VER         "VER"          // requisitar versão do aplicativo
#define SERVER_RX_OPENCVON     "OPENCVON"       // liga modo autonomo
#define SERVER_RX_OPENCVOFF    "OPENCVOFF"      // desliga modo autonomo      
#define SERVER_RX_MOVEUP      "MOVEUP"       // requisitar movimento para frente do robo
#define SERVER_RX_MOVEDOWN    "MOVEDOWN"     // requisitar movimento para tras do robo
#define SERVER_RX_MOVELEFT    "MOVELEFT"     // requisitar movimento para esquerda do robo
#define SERVER_RX_MOVERIGHT   "MOVERIGHT"    // requisitar movimento para direita do robo
#define SERVER_RX_STOP        "STOP"         // requisitar parada do robo
#define SERVER_RX_COLTOGGLE   "COLTOGGLE"    // requisitar toggle do estado do coletor

    /*
       pacotes que podem ser enviados
       */
#define SERVER_TX_VER         "VER"          // resposta ao pacote VER
#define SERVER_TX_OPENCVON     "OPENCVON"      // resposta ao pacote AUTOON
#define SERVER_TX_OPENCVOFF    "OPENCVOFF"     // resposta ao pacote AUTOOFF

#define SERVER_TX_COLLECTOR_RUN       "COLLECTOR_RUN"        // resposta ao pacote COLLECTOR_RUN
#define SERVER_TX_COLLECTOR_STOP      "COLLECTOR_STOP"       // resposta ao pacote COLLECTOR_STOP
#define SERVER_TX_COLLECTOR_TIMER     "COLLECTOR_TIMER"      // resposta ao pacote COLLECTOR_TIMER

    /*
       codigos de erros
       */
    //erros ocorridos ao enviar/receber comandos
#define	SERVER_ERR_OK                0  // ok
#define SERVER_ERR_NODEF            -1  // erro nao definido
#define	SERVER_ERR_TIMEOUT          -4  // timeout
#define SERVER_ERR_MANYARG          -8  // ultrapassou limite máximo de argumentos suportado
#define SERVER_ERR_NO_CONNECTED     -15 // não conectado ao client
    //erros ocorridos ao tentar conectar com o client
#define SERVER_ERR_CON_SOCK         -20 // falha ao tentar abrir o file descriptor do socket
    //erros enviados ao client
#define SERVER_ERR_CMD_INVALID      -30 // CMD: comando inválido
#define SERVER_ERR_CMD_INEXIST      -31 // CMD: comando inexistente
#define SERVER_ERR_CMD_ARG_INVALID  -32 // CMD: argumento inválido
#define SERVER_ERR_CMD_ARG_TOOMANY  -33 // CMD: excesso de argumentos
#define SERVER_ERR_CMD_ARG_TOOLONG  -34 // CMD: argumento muito extenso

    /*
       propriedades do servidor
       */
    struct server_st {
        int fd;                            // file descriptor do socket
        int port;                          // porta do servidor
        char client_addr[16];              // IP do client obtido após a conexão
        int  client_port;                  // porta do client obtida após a conexão
        int  local_port;                   // porta local obtida após a conexão
        int tmo_cnt;                       // contador de time-out ocorrido
    };


    /*
       public vars
       */
    extern struct server_st server;


    /*
       external
       */
    extern int server_start(void);

    extern int server_stop(void);

    extern int server_connect(void);

    extern int server_disconnect(void);

    extern int server_send_plain_text(char *s);

    extern int server_send_package(char * name, char *fields);


#ifdef  __cplusplus
}
#endif

#endif /* __SERVER_H__ */

