# Lambda-loader-so-hackaton

Link repository github: https://github.com/stefanstan2402/Lambda-loader-so-hackaton

Componenta echipei: Pantucu Stefan
                    Stan Stefan
                    
 Teste functionale: cele vizibile de pe checker: 1-6 + testele 10 si 11 care functoneaza la rularea manuala a acestora. Ni s-a explicat ca datorita testelor ce verifica paralelismul nu functioneaza acelea.
 Daca marim timeout de la 1s la 6s, functioneaza si testul 7.
 
 Ca si implementare, cerinta este implementata, doar ca secvential, iar pe langa partea de logica ceruta, am adaugat si partea de logging intr-un fisier, acest tip de logging facandu-se pe fiecare instanta de rulare a serverului. Multiplexarea I/O am facut-o cu epoll. 
