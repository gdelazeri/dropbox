./dropboxServer port type <primaryIP> <portIP>

port: 	Porta para comunica��o entre os servidores
type: 	--primary: Informa que o servidor � o prim�rio
	--backup: Informa que o servidor � o backup
<primaryIP> (s� para backup): IP do servidor prim�rio para comunica��o entre servidores
<portIP> (s� para backup): Porta do servidor prim�rio para comunica��o entre servidores

O socket principal do servidor seguir� sendo a constante 4000, dessa forma todos os clientes sempre ir�o se conectar na porta 4000