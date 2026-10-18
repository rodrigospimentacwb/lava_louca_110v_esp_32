# Fluxo de trabalho

- **Responder sempre em português do Brasil.** Comentários no código em português; nomes de funções, variáveis e arquivos em inglês (decisão do dono do projeto).
- **Toda alteração no repositório vai por pull request.** Nunca commitar direto na branch padrão. O dono revisa e faz o merge.
- **Nunca subir senhas.** Credenciais ficam só em `LavaLoucas/secrets.h` (no `.gitignore`). No git vai apenas `secrets.example.h` com valores de exemplo. Antes de qualquer commit, conferir que nenhum arquivo versionado tem senha.
  - A senha antiga do AP (`lavaloucas123`) vazou em conversas e deve ser tratada como pública; não reutilizar.
- **Não instalar nada na máquina do dono sem pedir.** Dizer o que falta e o comando.
- `sudo` não funciona pelo `!` do Claude Code (não há terminal para a senha): pedir para o dono rodar num terminal próprio.
- Antes de dizer que algo está pronto: compilar com `--warnings default` e confirmar zero erros e zero avisos.
- Nunca publicar o `.bin` compilado: a senha do AP vai embutida nele.
- **Tom da documentação:** humor só na apresentação do projeto (abertura do README). Avisos de perigo, instruções de ligação, configuração e segurança são sempre sérios e diretos: o projeto mexe com 110 V e água. O README é público: não citar pessoas da família.
