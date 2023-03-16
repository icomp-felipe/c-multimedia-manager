# Gerenciador Multimídia

- Criado por: Felipe André
- Lançamento: 01/01/2018
- Revisão   : 12/01/2018

# Changelog v.4.0

* Melhorias na estabilidade do sistema
* Utilização da biblioteca "libconfuse" para leitura e...
* ...Implementação do arquivo de propriedades do sistema (program.properties)
* Utilização de threads: não bloqueia o sistema enquanto se está convertendo
  um arquivo ou abrindo um diretório com o gerenciador de arquivos
* Leitura de configurações em tempo real para os seguintes atributos:
  * diretório de saída de mídia;
  * media player;
  * string de conversão de mídia;
* Removida a função de "último arquivo inserido"
* Renomeado arquivo de banco de dados para "database.dbx"
* Renomeado arquivo de backup de banco de dados para "database.bkp"
* Renomeado arquivo temporário de banco de dados para "database.tmp"
* Incluídas as bibliotecas {"autoDelete.h","propertiesManager.h","supportedMedia.h"}