[English](#english) | [Português (Brasil)](#portugues)

<a id="english"></a>

# Contributing

Start with BUILD, ARCHITECTURE and SOURCES. Use your own supported game copy; keep original resources, generated PowerPC bodies, saves and private traces outside Git.

For a bug, provide the release, hardware/driver, scene, steps and expected/actual behavior. Explain what you reproduced and what remains a hypothesis. Redact paths and account information. An isolated crash fix should include the original function boundary evidence and a focused reproduction when available.

For performance work, record the candidate's executable/GPU/runtime hashes, exact scene, input, resolution, cap, capture/profiler state and frame-time distribution. Compare with the saved accepted reference. Do not combine unrelated experiments, lower below native resolution to claim an optimization, count repeated presents as game frames, or claim a tiny change is a gain without evidence beyond noise. Keep only demonstrated improvements and inspect graphics after a worthwhile measured gain.

Keep SDK changes as one documented diff against the pinned base. Use the SDK's renderer/kernel rather than introducing replacement implementations. Game-specific PC menu integration belongs in the consumer project. Menu patches must validate original hashes, preserve backups and avoid touching saves.

For UI changes, exercise keyboard/controller navigation, normal/direct startup, error paths, the original game menu and the launcher return cycle. Test that the frontend has exited during gameplay. Do not enable Mods, Players or uncapped mode as placeholders.

Pull requests should state the problem, resulting behavior, actual tests and limitations. Include dependency origin/version/license changes. Never claim a clean-machine build, campaign completion or stable frame rate that was not verified.

---

<a id="portugues"></a>

# Como contribuir

Leia BUILD, ARCHITECTURE e SOURCES. Use sua própria cópia suportada e mantenha recursos originais, código PowerPC gerado, saves e traces privados fora do Git.

Relate versão, hardware/driver, cena e passos, separando o que foi reproduzido de hipóteses. Em otimização, registre hashes dos binários, cena, entrada, resolução, limite e distribuição dos tempos de quadro. Compare com a referência aceita salva; não reivindique ganhos dentro do ruído nem conte presents repetidos como quadros do jogo.

Mantenha um único diff do SDK contra a base fixada. A integração específica de menus fica no projeto consumidor, com hashes e backups, sem alterar saves. Teste controle/teclado, início normal/direto e retorno ao launcher. Documente resultados reais e limites; não envie arquivos do jogo ou dados pessoais.
