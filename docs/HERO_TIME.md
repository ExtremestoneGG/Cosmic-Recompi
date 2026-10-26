[English](#english) | [Português (Brasil)](#portugues)

<a id="english"></a>
# Hero Time — Alpha release notes

This Alpha adds **Options → Gameplay → Extras → Hero Time**, disabled by
default. Original Gameplay settings are available under **Standard**; All Aliens
is grouped under **Extras**. The menus use the game's original interface.

First transform into a compatible normal alien, then click the left stick
(**L3**, **F** with the default keyboard mapping) to request its corresponding
Ultimate form. Ben cannot transform directly into an Ultimate. Supported pairs
are Spider Monkey, Humungousaur, Echo Echo, Swampfire and Big Chill. Way Big and
Ultimate Way Big are excluded. No new character assets are distributed.

The special-energy meter shows up to **60 seconds** of Ultimate time. At expiry,
the game returns to Ben and recharges for **30 seconds** before normal player
transformations become available again. This release uses the fixed 30-second
recharge; proportional recharge and an additional green transformation flash
are planned separately and are not included.

## What was verified

- The user manually confirmed Ultimate attacks and the timer meter worked.
- Runtime events recorded Ultimate confirmation, return to Ben after 60.025
  seconds, and completion of a 30.000-second recharge in that test.
- Authored bytecode tests cover eligible pairs, readiness gates, cycle timing,
  resource accounting, death/story interruption and energy-meter values.
- Compiled installer tests cover script composition, branch destinations,
  repeated installation, unknown-edit rejection and preservation of test saves.

These checks do not certify every Ultimate ability, every mission or the full
campaign. Pending resource-load cancellation across mission changes requires
more testing. Back up saves before testing Alpha software.

## Known issues

- Ultimate Big Chill can show an opaque geometric border around its wings.
- Camera/scenery flicker above 30 FPS remains under investigation.
- Stable 60 FPS has not been established. No new FPS improvement is claimed.
- Existing issues, including Rath upgrades with All Aliens disabled, remain
  outside this change's validation scope.

Bring your own supported game copy. No ISO, XEX, original assets, generated
game source, private debug room, translation patch or personal saves are bundled.

---
<a id="portugues"></a>
# Hero Time — notas da versão Alpha

Adiciona **Options → Gameplay → Extras → Hero Time**, desligado por padrão.
As configurações originais ficam em **Standard** e All Aliens fica em **Extras**.
Os menus usam a interface do próprio jogo.

Transforme-se primeiro no alien normal compatível e clique no analógico
esquerdo (**L3**, ou **F** no teclado) para ativar o Supremo correspondente.
Ben não vira Supremo diretamente. Gigante e Gigante Supremo não participam.

O Supremo dura até **60 segundos**, usando a barra de energia como temporizador.
Depois, o personagem retorna ao Ben e recarrega por **30 segundos**. A recarga
proporcional e o brilho verde adicional ficam para outra atualização.

O usuário confirmou os golpes e a barra no teste manual. O log registrou
60,025 segundos até o retorno e 30 segundos de recarga. Isso não comprova todas
as habilidades, fases ou a campanha inteira. O contorno nas asas do Friagem
Supremo é um problema conhecido. A tremedeira acima de 30 FPS continua aberta;
não há ganho novo de FPS anunciado nem confirmação de 60 FPS estáveis.

É necessário fornecer sua própria cópia compatível. Nenhum dado original,
save pessoal ou sala privada de debug é distribuído.
