./dropboxServer port type <primaryIP> <portIP>

port: 	Porta para comunicação entre os servidores
type: 	--primary: Informa que o servidor é o primário
	--backup: Informa que o servidor é o backup
<primaryIP> (só para backup): IP do servidor primário para comunicação entre servidores
<portIP> (só para backup): Porta do servidor primário para comunicação entre servidores

O socket principal do servidor seguirá sendo a constante 4000, dessa forma todos os clientes sempre irão se conectar na porta 4000