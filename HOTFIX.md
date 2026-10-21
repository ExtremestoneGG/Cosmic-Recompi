[English](#english) | [Português (Brasil)](#portugues)

<a id="english"></a>

# Gameplay callback hotfix

The public test log identified two distinct failures. They must not be described as one confirmed fix.

## Corrected: missing callback 0x8210BB88

Automatic function discovery omitted a 12-byte callback outside the executable's unwind entries. The manifest now specifies its reviewed boundary. ReXGlue regenerates and registers it; its original behavior forwards the callback object, clears the argument and tail-calls the existing field setter. No replacement renderer or kernel is involved.

Validation covers 8,192 original-instruction oracle cases, registration in the real runtime, execution of the actual callback and callee on an isolated guest allocation, preservation of adjacent memory and LR/CTR, and a 180-second initial-game smoke test. The exact long player route was not replayed. Startup checks retain this callback and the previous callback fixes in later builds.

## Pending: dying at the first boss, target 0x00000000

The earlier log contains a null indirect-call failure. The player identified its trigger as dying during the first boss fight. The old log has no return address or stack, so it does not establish the caller or a safe correction. The current save is beyond that fight; it has not been rolled back.

This build adds diagnostics **only when an invalid dispatch already occurs**: guest return address/registers, safely readable stack candidates and the native call stack. It preserves the fatal behavior. It does not register a dummy null function or silently skip the failing call. The boss-death failure remains unconfirmed until its route is reproduced and repaired.

On recurrence, preserve `<installation>/logs/runtime.log` and report the action/scene. Review personal paths before sharing logs; do not upload saves or assets publicly.

GPU binaries, resolution presets, FPS limits and the retained optimization stack are unchanged. This is a correctness/diagnostic update, not a new performance-gain claim. Updates preserve `user`; installed-file replacements keep backups.

---

<a id="portugues"></a>

# Correção do callback de gameplay

O log contém duas falhas distintas. A função ausente `0x8210BB88` foi incluída no manifesto com seu limite original de 12 bytes, recompilada e registrada. Passou em 8.192 comparações com as instruções originais, execução real pelo dispatcher sobre uma alocação isolada e teste de 180 segundos. A rota longa exata do jogador não foi repetida.

O crash `0x00000000`, identificado pelo jogador ao morrer no primeiro chefe, continua em investigação. O log antigo não contém retorno/pilha suficiente para identificar a origem. O save atual passou desse chefe e foi preservado, sem retrocesso.

O novo runtime registra contexto somente se a chamada inválida acontecer: registradores/retorno, candidatos da pilha do jogo lidos com segurança e pilha nativa. A falha continua fatal; nenhuma chamada nula é ignorada ou substituída por uma função vazia. Ainda não há confirmação de correção desse segundo crash.

Se repetir, preserve `<instalação>/logs/runtime.log` e informe a ação/cena. Remova caminhos pessoais antes de compartilhar e não publique saves ou recursos originais.

O DLL da GPU, resolução, limites de FPS e otimizações acumuladas permanecem iguais. Não há promessa de ganho novo de desempenho. A atualização preserva `user` e mantém backups dos arquivos substituídos.
