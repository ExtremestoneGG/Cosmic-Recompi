[English](#english) | [Português (Brasil)](#portugues)

<a id="english"></a>
# Cosmic Recomp Alpha 0.4.0-alpha.4 — stability and accumulated optimizations

This is an **Alpha update**, not a completed port. Bring your own supported Xbox
360 copy. The importer prepares local game data and uses the bundled native
runtime; it does not compile C++ on the player's machine.

## Changes

- Increased the DB File Heap from 64 to **96 MiB** for the additional form resources.
  Both callback record tables and their auxiliary queues now hold **64 entries**
  in appended storage. Existing object fields retain their offsets. These changes
  address missing Vreedle weapons/effects and the crash at the battle-to-story
  transition with All Aliens and Hero Time enabled. The encounter was manually
  retested successfully; this is not full-campaign certification.
- **Hero Time:** the normal-to-Ultimate request now uses the original green
  transformation effect. Ultimate time remains 60 seconds. Returning early
  preserves the corresponding meter fraction; recharge is **time used / 3**, up
  to **20 seconds** as Ben. For example, 30 seconds used means 10 seconds recharge.
  The user confirmed the effect, early return and proportional meter behavior.
- Accumulated native-quality performance work: source-pitch specialization in
  eligible depth resolves, guarded RGBA8 resolve copies into existing compatible
  textures, and bounded CPU PAUSE hints while GPU progress remains unchanged.
  Unsupported transfer layouts retain the original SDK path. Polling keeps all
  guest checks and deadlines; it does not change game speed or impose a frame cap.
- The FPS overlay also reports process RAM and DXGI GPU-memory usage. GPU shared
  memory on integrated graphics is not additional dedicated VRAM.
- Updated window title, complete corresponding SDK sources, one consolidated SDK
  diff, and authored post-codegen integration recipes. No original game code or
  assets are included.

## Performance and limitations

The eight-hint polling candidate measured **39.919 vs 38.940 game FPS (+2.52%)**
against a stored reference in one fixed native-720p Paris scene. The same private
camera instrumentation was present on both sides. P95 changed from 34.821 to
34.908 ms; P99 from 44.096 to 35.563 ms. This is a scoped development observation,
**not a release-to-release guarantee** or proof of stable 60 FPS. Private camera
experiments are excluded from this release.

**30 FPS remains the default. Camera/scenery flicker above 30 FPS is still open.**
Ultimate Big Chill's wing transparency and Rath upgrades with All Aliens disabled
also remain open. No frame generation, FSR, new mouse wheel UI or private test
room is shipped. Existing controls, original-game video menus, save support and
the separate launcher remain available. Back up your user folder before updating.

Validation separates automated recipe/VM/installer checks from manual encounter
and Hero Time feedback. See VALIDATION.json and docs/ALPHA4.md. A clean-machine
rebuild of the entire SDK and consumer has not been verified.

---
<a id="portugues"></a>
# Cosmic Recomp Alpha 0.4.0-alpha.4 — estabilidade e otimizações acumuladas

Versão **Alpha**. É necessário fornecer sua própria cópia compatível de Xbox 360.
O importador prepara os dados locais usando o runtime nativo distribuído; não
compila C++ na máquina do jogador.

- Pool DB File Heap ampliado de 64 para **96 MiB**, com ambas as listas de
  callbacks e filas auxiliares ampliadas para **64 entradas**. Corrige a falta de
  armas/efeitos dos Vreedle e o crash na transição após a batalha com All Aliens e
  Hero Time. Essa luta foi retestada manualmente; a campanha inteira não foi validada.
- **Hero Time:** brilho verde original, até 60 segundos como Supremo e recarga
  proporcional ao uso, limitada a **20 segundos**. Usar 30 segundos resulta em
  10 segundos de recarga. Saída antecipada e barra proporcional foram confirmadas.
- Otimizações acumuladas de resolves de profundidade, cópias RGBA8 compatíveis
  para texturas existentes e espera da CPU por progresso da GPU, preservando
  resolução nativa, verificações e caminhos de fallback do SDK.
- Contador com RAM do processo e memória de GPU via DXGI. Memória compartilhada
  de uma GPU integrada não equivale a VRAM dedicada adicional.
- Fontes correspondentes completos do SDK, diff consolidado e receitas de
  integração após codegen. Nenhum código gerado ou arquivo original do jogo.

A observação de **+2,52%** é restrita a uma cena de desenvolvimento comparada ao
registro equivalente; não representa ganho garantido entre versões públicas.
**30 FPS segue como padrão. O flicker acima de 30 FPS ainda não está corrigido.**
Transparência das asas do Friagem Supremo e upgrades do Rath com All Aliens
desligado continuam pendentes. Não inclui framegen, FSR ou sala privada de testes.
Preserve sua pasta de usuário ao atualizar. Consulte VALIDATION.json para separar
testes automáticos, confirmações manuais e pontos ainda não verificados.
