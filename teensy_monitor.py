import tkinter as tk
from tkinter import ttk, messagebox
import serial
import serial.tools.list_ports
import threading
import time
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import numpy as np
import queue


class TeensyMonitorApp:
	def __init__(self, root):
		self.root = root
		self.root.title("Teensy Feedback Canceller Monitor")
		self.root.geometry("1000x700")

		# Variables
		self.serial_port = None
		self.is_connected = False
		self.reading_thread = None
		self.should_stop = False
		self.data_queue = queue.Queue()

		# Historique des données
		self.time_data = np.array([])
		self.freq_data = np.array([])
		self.amplitude_data = np.array([])
		self.max_points = 200  # Nombre de points à afficher sur le graphique

		# État du système
		self.mode_state = "INACTIF"
		self.current_freq = 0.0
		self.current_gain = 1.0
		self.lms_enabled = True
		self.notch_enabled = False
		self.muted = False

		# Création de l'interface
		self.create_widgets()

		# Mise à jour automatique de l'interface
		self.update_timer()

	def create_widgets(self):
		# Frame principal
		main_frame = ttk.Frame(self.root, padding="10")
		main_frame.pack(fill=tk.BOTH, expand=True)

		# Section connexion
		conn_frame = ttk.LabelFrame(main_frame, text="Connexion", padding="10")
		conn_frame.pack(fill=tk.X, pady=5)

		ttk.Label(conn_frame, text="Port:").grid(row=0, column=0, padx=5, pady=5, sticky=tk.W)

		self.port_combo = ttk.Combobox(conn_frame, width=30)
		self.port_combo.grid(row=0, column=1, padx=5, pady=5)

		self.refresh_btn = ttk.Button(conn_frame, text="Rafraîchir", command=self.refresh_ports)
		self.refresh_btn.grid(row=0, column=2, padx=5, pady=5)

		self.connect_btn = ttk.Button(conn_frame, text="Connecter", command=self.toggle_connection)
		self.connect_btn.grid(row=0, column=3, padx=5, pady=5)

		# Section état et contrôles
		controls_frame = ttk.LabelFrame(main_frame, text="Contrôles et État du système", padding="10")
		controls_frame.pack(fill=tk.X, pady=5)

		# Première ligne: Mode et fréquence dominante
		ttk.Label(controls_frame, text="Mode:").grid(row=0, column=0, padx=5, pady=5, sticky=tk.W)
		self.mode_label = ttk.Label(controls_frame, text="INACTIF", foreground="red")
		self.mode_label.grid(row=0, column=1, padx=5, pady=5, sticky=tk.W)

		ttk.Label(controls_frame, text="Fréquence dominante:").grid(row=0, column=2, padx=5, pady=5, sticky=tk.W)
		self.freq_label = ttk.Label(controls_frame, text="0.0 Hz")
		self.freq_label.grid(row=0, column=3, padx=5, pady=5, sticky=tk.W)

		# Deuxième ligne: Gain
		ttk.Label(controls_frame, text="Gain:").grid(row=1, column=0, padx=5, pady=5, sticky=tk.W)

		# Frame pour slider de gain et sa valeur
		gain_frame = ttk.Frame(controls_frame)
		gain_frame.grid(row=1, column=1, columnspan=3, padx=5, pady=5, sticky=tk.W+tk.E)

		self.gain_value = tk.DoubleVar(value=1.0)
		self.gain_slider = ttk.Scale(gain_frame, from_=0.0, to=5.0, orient=tk.HORIZONTAL,
		                             variable=self.gain_value, command=self.on_gain_change)
		self.gain_slider.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=5)

		self.gain_label = ttk.Label(gain_frame, text="1.0")
		self.gain_label.pack(side=tk.RIGHT, padx=5)

		# Troisième ligne: Filtres
		filters_frame = ttk.Frame(controls_frame)
		filters_frame.grid(row=2, column=0, columnspan=4, padx=5, pady=10, sticky=tk.W+tk.E)

		# Variables pour les états des boutons
		self.lms_var = tk.BooleanVar(value=True)
		self.notch_var = tk.BooleanVar(value=False)
		self.mute_var = tk.BooleanVar(value=False)

		# Bouton LMS
		self.lms_btn = ttk.Checkbutton(filters_frame, text="Filtre LMS",
		                               variable=self.lms_var, command=self.toggle_lms)
		self.lms_btn.pack(side=tk.LEFT, padx=10)

		# Bouton Notch
		self.notch_btn = ttk.Checkbutton(filters_frame, text="Filtre Notch",
		                                 variable=self.notch_var, command=self.toggle_notch)
		self.notch_btn.pack(side=tk.LEFT, padx=10)

		# Bouton Mute
		self.mute_btn = ttk.Checkbutton(filters_frame, text="Mute",
		                                variable=self.mute_var, command=self.toggle_mute)
		self.mute_btn.pack(side=tk.LEFT, padx=10)

		# Bouton Reset LMS
		self.reset_lms_btn = ttk.Button(filters_frame, text="Reset LMS", command=self.reset_lms)
		self.reset_lms_btn.pack(side=tk.LEFT, padx=20)

		# Bouton pour demander le statut
		self.get_status_btn = ttk.Button(filters_frame, text="Obtenir le statut", command=self.get_status)
		self.get_status_btn.pack(side=tk.RIGHT, padx=10)

		# Section pour les indicateurs visuels de l'état des filtres
		indicators_frame = ttk.LabelFrame(main_frame, text="État des filtres", padding="10")
		indicators_frame.pack(fill=tk.X, pady=5)

		# Créer un cadre pour aligner les indicateurs
		ind_inner_frame = ttk.Frame(indicators_frame)
		ind_inner_frame.pack(fill=tk.X, pady=5)

		# Indicateurs d'état avec des étiquettes
		ttk.Label(ind_inner_frame, text="LMS:").grid(row=0, column=0, padx=5, pady=5)
		self.lms_indicator = ttk.Label(ind_inner_frame, text="ACTIVÉ", foreground="green", width=10)
		self.lms_indicator.grid(row=0, column=1, padx=5, pady=5)

		ttk.Label(ind_inner_frame, text="Notch:").grid(row=0, column=2, padx=5, pady=5)
		self.notch_indicator = ttk.Label(ind_inner_frame, text="DÉSACTIVÉ", foreground="red", width=10)
		self.notch_indicator.grid(row=0, column=3, padx=5, pady=5)

		ttk.Label(ind_inner_frame, text="Mute:").grid(row=0, column=4, padx=5, pady=5)
		self.mute_indicator = ttk.Label(ind_inner_frame, text="DÉSACTIVÉ", foreground="red", width=10)
		self.mute_indicator.grid(row=0, column=5, padx=5, pady=5)

		# Graphique
		graph_frame = ttk.LabelFrame(main_frame, text="Analyse spectrale", padding="10")
		graph_frame.pack(fill=tk.BOTH, expand=True, pady=5)

		self.fig, (self.ax1, self.ax2) = plt.subplots(2, 1, figsize=(9, 6))
		self.canvas = FigureCanvasTkAgg(self.fig, master=graph_frame)
		self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

		# Graphique de fréquence dominante
		self.ax1.set_title("Fréquence dominante")
		self.ax1.set_xlabel("Temps (s)")
		self.ax1.set_ylabel("Fréquence (Hz)")
		self.freq_line, = self.ax1.plot([], [], 'b-')
		self.ax1.grid(True)

		# Graphique d'amplitude
		self.ax2.set_title("Amplitude du signal")
		self.ax2.set_xlabel("Temps (s)")
		self.ax2.set_ylabel("Amplitude")
		self.amp_line, = self.ax2.plot([], [], 'r-')
		self.ax2.grid(True)

		self.fig.tight_layout()

		# Console de logs
		log_frame = ttk.LabelFrame(main_frame, text="Console de logs", padding="5")
		log_frame.pack(fill=tk.X, pady=5)

		self.log_text = tk.Text(log_frame, height=5, wrap=tk.WORD)
		self.log_text.pack(fill=tk.X, padx=5, pady=5)

		# Scrollbar pour la console
		scrollbar = ttk.Scrollbar(self.log_text, command=self.log_text.yview)
		scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
		self.log_text.config(yscrollcommand=scrollbar.set)

		# Status bar
		self.status_var = tk.StringVar()
		self.status_var.set("Prêt. Connectez-vous à une Teensy.")
		status_bar = ttk.Label(main_frame, textvariable=self.status_var, relief=tk.SUNKEN, anchor=tk.W)
		status_bar.pack(fill=tk.X, side=tk.BOTTOM, pady=5)

		# Initialiser la liste des ports
		self.refresh_ports()

		# Désactiver les contrôles jusqu'à la connexion
		self.set_controls_state(tk.DISABLED)

	def set_controls_state(self, state):
		"""Active ou désactive les contrôles en fonction de l'état de connexion"""
		self.gain_slider.config(state=state)
		self.lms_btn.config(state=state)
		self.notch_btn.config(state=state)
		self.mute_btn.config(state=state)
		self.reset_lms_btn.config(state=state)
		self.get_status_btn.config(state=state)

	def refresh_ports(self):
		"""Rafraîchit la liste des ports série disponibles"""
		ports = [port.device for port in serial.tools.list_ports.comports()]
		self.port_combo['values'] = ports
		if ports:
			self.port_combo.current(0)

	def toggle_connection(self):
		"""Connecte ou déconnecte le port série"""
		if not self.is_connected:
			self.connect_serial()
		else:
			self.disconnect_serial()

	def connect_serial(self):
		"""Établit la connexion série avec la Teensy"""
		port = self.port_combo.get()
		if not port:
			messagebox.showerror("Erreur", "Veuillez sélectionner un port.")
			return

		try:
			self.serial_port = serial.Serial(port, 115200, timeout=1)
			self.is_connected = True
			self.connect_btn.config(text="Déconnecter")
			self.status_var.set(f"Connecté à {port}")

			# Activer les contrôles
			self.set_controls_state(tk.NORMAL)

			# Lancer le thread de lecture
			self.should_stop = False
			self.reading_thread = threading.Thread(target=self.read_serial_data)
			self.reading_thread.daemon = True
			self.reading_thread.start()

			# Demander l'état actuel
			self.get_status()

			self.log("Connexion établie avec succès")

		except Exception as e:
			messagebox.showerror("Erreur de connexion", str(e))
			self.log(f"Erreur de connexion: {str(e)}")

	def disconnect_serial(self):
		"""Déconnecte le port série"""
		if self.serial_port and self.serial_port.is_open:
			self.should_stop = True
			if self.reading_thread:
				self.reading_thread.join(timeout=1.0)
			self.serial_port.close()

		self.is_connected = False
		self.connect_btn.config(text="Connecter")
		self.status_var.set("Déconnecté.")

		# Désactiver les contrôles
		self.set_controls_state(tk.DISABLED)

		self.log("Déconnexion effectuée")

	def send_command(self, command):
		"""Envoie une commande au Teensy"""
		if self.serial_port and self.serial_port.is_open:
			try:
				self.serial_port.write((command + "\n").encode('utf-8'))
				self.log(f"Commande envoyée: {command}")
				return True
			except Exception as e:
				self.log(f"Erreur lors de l'envoi de la commande: {str(e)}")
				return False
		else:
			self.log("Impossible d'envoyer la commande: non connecté")
			return False

	def on_gain_change(self, event=None):
		"""Gère le changement de gain via le slider"""
		gain = self.gain_value.get()
		self.gain_label.config(text=f"{gain:.2f}")

		# Ne pas envoyer à chaque petit changement pendant le drag
		if not event or isinstance(event, str):  # Si l'événement est une chaîne, c'est un relâchement du slider
			self.current_gain = gain
			self.send_command(f"SET:GAIN:{gain:.2f}")

	def toggle_lms(self):
		"""Active ou désactive le filtre LMS"""
		state = self.lms_var.get()
		command = "SET:LMS:ON" if state else "SET:LMS:OFF"
		if self.send_command(command):
			self.lms_enabled = state
			self.update_indicators()

	def toggle_notch(self):
		"""Active ou désactive le filtre Notch"""
		state = self.notch_var.get()
		command = "SET:NOTCH:ON" if state else "SET:NOTCH:OFF"
		if self.send_command(command):
			self.notch_enabled = state
			self.update_indicators()

	def toggle_mute(self):
		"""Active ou désactive le mute"""
		state = self.mute_var.get()
		command = "SET:MUTE:ON" if state else "SET:MUTE:OFF"
		if self.send_command(command):
			self.muted = state
			self.update_indicators()

	def reset_lms(self):
		"""Réinitialise le filtre LMS"""
		self.send_command("RESET:LMS")

	def get_status(self):
		"""Demande le statut actuel du système"""
		self.send_command("GET:STATUS")

	def update_indicators(self):
		"""Met à jour les indicateurs visuels de l'état des filtres"""
		# LMS
		self.lms_indicator.config(
			text="ACTIVÉ" if self.lms_enabled else "DÉSACTIVÉ",
			foreground="green" if self.lms_enabled else "red"
		)

		# Notch
		self.notch_indicator.config(
			text="ACTIVÉ" if self.notch_enabled else "DÉSACTIVÉ",
			foreground="green" if self.notch_enabled else "red"
		)

		# Mute
		self.mute_indicator.config(
			text="ACTIVÉ" if self.muted else "DÉSACTIVÉ",
			foreground="red" if self.muted else "green"  # Inversé car mute activé est généralement indiqué en rouge
		)

	def read_serial_data(self):
		"""Thread pour lire les données série"""
		while not self.should_stop:
			try:
				if self.serial_port and self.serial_port.is_open:
					line = self.serial_port.readline().decode('utf-8').strip()
					if line:
						# Mettre les données dans la queue pour traitement dans le thread principal
						self.data_queue.put(line)
			except Exception as e:
				self.log(f"Erreur de lecture: {e}")
				time.sleep(0.1)  # Éviter de surcharger le CPU en cas d'erreur

	def process_data(self, data_line):
		"""Traite les données reçues de la Teensy"""
		self.log(f"Reçu: {data_line}")

		if not data_line.startswith("DATA:"):
			return

		parts = data_line.split(":", 2)
		if len(parts) < 3:
			return

		data_type = parts[1]
		data_value = parts[2]

		if data_type == "INIT":
			self.status_var.set(f"Connecté et initialisé: {data_value}")

		elif data_type == "MODE":
			self.mode_state = data_value
			self.mode_label.config(
				text=data_value,
				foreground="green" if data_value == "ACTIF" else "red"
			)

		elif data_type == "FREQ":
			try:
				values = data_value.split(",")
				freq = float(values[0])
				amplitude = float(values[1]) if len(values) > 1 else 0.0

				# Mettre à jour les labels
				self.current_freq = freq
				self.freq_label.config(text=f"{freq:.1f} Hz")

				# Ajouter au graphique
				current_time = time.time()

				# Gérer l'initialisation des données
				if len(self.time_data) == 0:
					self.start_time = current_time

				relative_time = current_time - self.start_time

				# Ajouter les nouvelles données
				self.time_data = np.append(self.time_data, relative_time)
				self.freq_data = np.append(self.freq_data, freq)
				self.amplitude_data = np.append(self.amplitude_data, amplitude)

				# Limiter le nombre de points
				if len(self.time_data) > self.max_points:
					self.time_data = self.time_data[-self.max_points:]
					self.freq_data = self.freq_data[-self.max_points:]
					self.amplitude_data = self.amplitude_data[-self.max_points:]

			except Exception as e:
				self.log(f"Erreur de traitement des données: {e}")

		elif data_type == "GAIN":
			try:
				gain = float(data_value)
				self.current_gain = gain
				self.gain_value.set(gain)
				self.gain_label.config(text=f"{gain:.2f}")
			except Exception as e:
				self.log(f"Erreur de traitement du gain: {e}")

		elif data_type == "LMS":
			if data_value == "ON":
				self.lms_enabled = True
				self.lms_var.set(True)
			elif data_value == "OFF":
				self.lms_enabled = False
				self.lms_var.set(False)
			elif data_value == "RESET":
				self.log("Le filtre LMS a été réinitialisé")

			self.update_indicators()

		elif data_type == "NOTCH":
			if data_value == "ON":
				self.notch_enabled = True
				self.notch_var.set(True)
			elif data_value == "OFF":
				self.notch_enabled = False
				self.notch_var.set(False)

			self.update_indicators()

		elif data_type == "MUTE":
			if data_value == "ON":
				self.muted = True
				self.mute_var.set(True)
			elif data_value == "OFF":
				self.muted = False
				self.mute_var.set(False)

			self.update_indicators()

		elif data_type == "STATUS":
			try:
				status_parts = data_value.split(",")
				for part in status_parts:
					key_value = part.split(":")
					if len(key_value) == 2:
						key, value = key_value
						if key == "LMS":
							self.lms_enabled = (value == "ON")
							self.lms_var.set(self.lms_enabled)
						elif key == "NOTCH":
							self.notch_enabled = (value == "ON")
							self.notch_var.set(self.notch_enabled)
						elif key == "MUTE":
							self.muted = (value == "ON")
							self.mute_var.set(self.muted)

				self.update_indicators()

			except Exception as e:
				self.log(f"Erreur lors du traitement du statut: {e}")

	def update_plots(self):
		"""Met à jour les graphiques avec les données actuelles"""
		if len(self.time_data) > 0:
			# Mise à jour du graphique de fréquence
			self.freq_line.set_data(self.time_data, self.freq_data)
			self.ax1.relim()
			self.ax1.autoscale_view()

			# Mise à jour du graphique d'amplitude
			self.amp_line.set_data(self.time_data, self.amplitude_data)
			self.ax2.relim()
			self.ax2.autoscale_view()

			# Redessiner le canvas
			self.canvas.draw_idle()

	def log(self, message):
		"""Ajoute un message à la console de logs"""
		timestamp = time.strftime("%H:%M:%S")
		log_entry = f"[{timestamp}] {message}\n"

		self.log_text.configure(state=tk.NORMAL)
		self.log_text.insert(tk.END, log_entry)
		self.log_text.see(tk.END)  # Scroll to the end
		self.log_text.configure(state=tk.DISABLED)

	def update_timer(self):
		"""Fonction appelée régulièrement pour mettre à jour l'interface"""
		# Traiter les données en attente
		while not self.data_queue.empty():
			data = self.data_queue.get()
			self.process_data(data)

		# Mettre à jour les graphiques
		self.update_plots()

		# Réappeler cette fonction après 100ms
		self.root.after(100, self.update_timer)


if __name__ == "__main__":
	root = tk.Tk()
	app = TeensyMonitorApp(root)
	root.mainloop()