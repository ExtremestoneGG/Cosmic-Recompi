using System;
using System.Drawing;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace CosmicRecompi {
    internal sealed class Launcher : Form {
        readonly Label status = new Label();
        readonly ProgressBar progress = new ProgressBar();
        readonly Button select = new Button();
        readonly Button cancel = new Button();
        readonly Button play = new Button();
        readonly CheckBox uncapped = new CheckBox();
        readonly Button saves = new Button();
        System.Diagnostics.Process game;
        CancellationTokenSource cancellation;
        readonly string root = AppDomain.CurrentDomain.BaseDirectory;
        public Launcher() {
            Text = "Cosmic Recompi — preparar sua cópia";
            ClientSize = new Size(700, 410); MinimumSize = new Size(640, 450);
            BackColor = Color.FromArgb(23, 26, 31); ForeColor = Color.White;
            Font = new Font("Segoe UI", 11); StartPosition = FormStartPosition.CenterScreen;
            Label title = new Label { Text = "COSMIC RECOMPI", AutoSize = true, Location = new Point(28, 24), Font = new Font("Segoe UI", 21, FontStyle.Bold) };
            Label intro = new Label { Text = "Selecione sua própria ISO de Ben 10: Cosmic Destruction (Xbox 360).\nOs recursos serão importados localmente. Nenhum jogo acompanha este programa.", Location = new Point(30, 80), Size = new Size(630, 55), Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right };
            status.Text = "Pronto para verificar sua ISO. Esta build suporta uma edição específica do jogo.";
            status.SetBounds(30, 145, 630, 48); status.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
            progress.SetBounds(30, 200, 630, 20); progress.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
            select.Text = "Selecionar ISO e preparar"; select.SetBounds(30, 240, 275, 38);
            cancel.Text = "Cancelar"; cancel.SetBounds(320, 240, 130, 38); cancel.Enabled = false;
            play.Text = "Jogar — 30 FPS"; play.SetBounds(30, 292, 275, 38); play.Enabled = InstalledGame.Ready(root);
            uncapped.Text = "FPS ilimitado (indisponível)"; uncapped.SetBounds(320, 295, 330, 30); uncapped.Enabled = false;
            uncapped.CheckedChanged += delegate { play.Text = uncapped.Checked ? "Jogar — FPS ilimitado" : "Jogar — 30 FPS"; };
            saves.Text = "Abrir pasta de saves"; saves.SetBounds(30, 345, 275, 38);
            saves.Click += delegate {
                string folder = Path.Combine(InstalledGame.Installation(root), "user");
                if (Directory.Exists(folder)) System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo(folder) { UseShellExecute = true });
                else MessageBox.Show(this, "A pasta será criada ao preparar sua ISO.", Text);
            };
            play.Click += delegate {
                try {
                    game = InstalledGame.Start(root, uncapped.Checked);
                    play.Enabled = false; select.Enabled = false;
                    game.Exited += delegate { if (!IsDisposed && IsHandleCreated) BeginInvoke((Action)delegate { play.Enabled = true; select.Enabled = true; status.Text = "Jogo encerrado. Seus saves permanecem na pasta installation/user."; }); };
                    game.EnableRaisingEvents = true;
                    status.Text = "Jogo em execução. FPS e frame time aparecem dentro do jogo.";
                } catch (Exception error) { MessageBox.Show(this, error.Message, "Não foi possível iniciar"); }
            };
            select.Click += async delegate { await Prepare(); };
            cancel.Click += delegate { if (cancellation != null) cancellation.Cancel(); };
            FormClosing += delegate(object sender, FormClosingEventArgs e) {
                if (cancellation != null) { e.Cancel = true; cancellation.Cancel(); }
                if (game != null && !game.HasExited) { e.Cancel = true; WindowState = FormWindowState.Minimized; }
            };
            Controls.AddRange(new Control[] { title, intro, status, progress, select, cancel, play, uncapped, saves });
            if (InstalledGame.Ready(root)) status.Text = "Sua cópia está preparada. Saves em installation/user; padrão: 30 FPS.";
        }
        async Task Prepare() {
            if (!Path.GetPathRoot(root).Equals("D:\\", StringComparison.OrdinalIgnoreCase)) {
                MessageBox.Show(this, "Extraia este programa para o disco D: antes de continuar.", Text); return;
            }
            using (OpenFileDialog dialog = new OpenFileDialog { Filter = "Imagem Xbox 360 (*.iso)|*.iso", Title = "Selecione sua própria ISO", CheckFileExists = true, AddExtension = false, RestoreDirectory = true }) {
                if (dialog.ShowDialog(this) != DialogResult.OK) return;
                if (!Path.GetPathRoot(dialog.FileName).Equals("D:\\", StringComparison.OrdinalIgnoreCase)) {
                    MessageBox.Show(this, "Selecione uma ISO no disco D:.", Text); return;
                }
                string installation = Path.Combine(root, "installation");
                string staging = Path.Combine(installation, "import-" + Guid.NewGuid().ToString("N"));
                string destination = Path.Combine(installation, "game");
                if (Directory.Exists(destination)) { MessageBox.Show(this, "Já existe uma importação. Ela foi preservada; não será sobrescrita.", Text); return; }
                cancellation = new CancellationTokenSource(); select.Enabled = false; cancel.Enabled = true;
                DateTime last = DateTime.MinValue;
                IProgress<Tuple<long, long, string>> updates = new Progress<Tuple<long, long, string>>(delegate(Tuple<long, long, string> p) {
                    progress.Value = p.Item2 == 0 ? 0 : (int)Math.Min(100, p.Item1 * 100 / p.Item2);
                    status.Text = "Importando " + progress.Value + "% — " + p.Item3;
                });
                try {
                    status.Text = "Verificando estrutura e edição da ISO…";
                    await Task.Run(delegate {
                        InstalledGame.VerifyRuntime(root);
                        using (IsoImage image = new IsoImage(dialog.FileName)) {
                            image.ExtractFresh(staging, cancellation.Token, delegate(long done, long total, string file) {
                                DateTime now = DateTime.UtcNow;
                                if ((now - last).TotalMilliseconds >= 200 || done == total) { last = now; updates.Report(Tuple.Create(done, total, file)); }
                            });
                        }
                        cancellation.Token.ThrowIfCancellationRequested();
                        Directory.Move(staging, destination);
                        InstalledGame.CompleteImport(root);
                    });
                    progress.Value = 100;
                    status.Text = "ISO importada. Recursos locais prontos; os saves serão mantidos separados na pasta user.";
                    play.Enabled = InstalledGame.Ready(root);
                } catch (OperationCanceledException) {
                    status.Text = "Cancelado. Arquivos parciais preservados em " + staging;
                } catch (Exception error) {
                    status.Text = "Não foi possível preparar o jogo.";
                    MessageBox.Show(this, error.Message, "Falha na preparação", MessageBoxButtons.OK, MessageBoxIcon.Error);
                } finally { cancellation.Dispose(); cancellation = null; select.Enabled = true; cancel.Enabled = false; }
            }
        }
        [STAThread] static void Main(string[] args) {
            string root = AppDomain.CurrentDomain.BaseDirectory;
            if (!Path.GetPathRoot(root).Equals("D:\\", StringComparison.OrdinalIgnoreCase)) { MessageBox.Show("Extraia o programa para o disco D:.", "Cosmic Recompi"); return; }
            string temp = Path.Combine(root, "temp"); Directory.CreateDirectory(temp);
            Environment.SetEnvironmentVariable("TEMP", temp); Environment.SetEnvironmentVariable("TMP", temp);
            Application.EnableVisualStyles(); Application.SetCompatibleTextRenderingDefault(false);
            string identity;
            using (var sha = System.Security.Cryptography.SHA256.Create()) identity = BitConverter.ToString(sha.ComputeHash(System.Text.Encoding.UTF8.GetBytes(root.ToUpperInvariant()))).Replace("-", "");
            using (Mutex instance = new Mutex(false, "Local\\CosmicRecompi-" + identity)) {
                bool owned;
                try { owned = instance.WaitOne(0); } catch (AbandonedMutexException) { owned = true; }
                if (!owned) { MessageBox.Show("O launcher desta instalação já está aberto.", "Cosmic Recompi"); return; }
                try { using (Launcher window = new Launcher()) {
                if (args.Length == 1 && args[0] == "--render-test") {
                    window.Show(); window.Refresh();
                    using (Bitmap bitmap = new Bitmap(window.Width, window.Height)) {
                        window.DrawToBitmap(bitmap, new Rectangle(Point.Empty, bitmap.Size));
                        bitmap.Save(Path.Combine(root, "launcher-preview.png"));
                    }
                    return;
                }
                Application.Run(window);
                } } finally { instance.ReleaseMutex(); }
            }
        }
    }
}
