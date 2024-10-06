import numpy as np

# ==================================
# Information
# ==================================
# Function call ```./write/new_enc/oneangle --use_opendata --use_deltaR --use_pt --weights 1 --minbin -6 --n_events 10000 --nbins 50 --file_prefix 10k_50bins ```

# Number of events:
n_events = 10000
# Process
energy = 14000
level = "data"
pid_1, pid_2 = 2212, 2212
outstate_str = "qcd"
# Weight information:
weight = (1)
# CMS 2011A Jet Dataset:
jet_alg = "anti-kt"
jet_scheme = "E?"
jet_rad = 0.5

# ==================================
# Output Histogram:
# ==================================
theta1_edges = [
	0, 1e-06, 1.39905e-06, 1.95734e-06, 2.73842e-06, 3.83119e-06, 5.36002e-06, 7.49894e-06, 1.04914e-05, 1.4678e-05, 2.05353e-05, 2.87298e-05, 4.01945e-05, 5.62341e-05, 7.86744e-05, 0.000110069, 0.000153993, 0.000215443, 0.000301416, 0.000421697, 0.000589975, 0.000825404, 0.00115478, 0.0016156, 0.0022603, 0.00316228, 0.00442419, 0.00618966, 0.00865964, 0.0121153, 0.0169499, 0.0237137, 0.0331767, 0.0464159, 0.0649382, 0.0908518, 0.127106, 0.177828, 0.24879, 0.34807, 0.486968, 0.681292, 0.953162, 1.33352, 1.86566, 2.61016, 3.65174, 5.10897, 7.14771, 10, np.inf
]

theta1_centers = [
	0, 1.18281e-06, 1.65482e-06, 2.31517e-06, 3.23904e-06, 4.53158e-06, 6.33991e-06, 8.86986e-06, 1.24094e-05, 1.73613e-05, 2.42894e-05, 3.39821e-05, 4.75426e-05, 6.65146e-05, 9.30572e-05, 0.000130192, 0.000182145, 0.00025483, 0.00035652, 0.000498789, 0.000697831, 0.0009763, 0.00136589, 0.00191095, 0.00267352, 0.00374039, 0.00523299, 0.00732122, 0.0102428, 0.0143301, 0.0200486, 0.028049, 0.0392419, 0.0549014, 0.0768098, 0.107461, 0.150343, 0.210337, 0.294273, 0.411702, 0.575992, 0.805842, 1.12741, 1.57731, 2.20673, 3.08733, 4.31933, 6.04296, 8.45441, np.inf
]

hist = [
	0.120419361, 0, 0, 0, 0, 0, 0, 0, 6.560836272e-05, 2.55414848e-05, 1.494125041e-05, 3.206347067e-05, 1.075746634e-05, 0.0001175844894, 8.759098938e-05, 0.000271716106, 0.0002097984233, 0.0005988219538, 0.00148096732, 0.00279727535, 0.01031585663, 0.01798275413, 0.03621511334, 0.06062724673, 0.1021477289, 0.1732381714, 0.2672715446, 0.3680256683, 0.4397204387, 0.4888945523, 0.5048607862, 0.5006496065, 0.4780831287, 0.4424621996, 0.425298819, 0.4059108978, 0.3914299139, 0.3813424388, 0.3553612933, 0.1884016603, 0.01182870093, 0.0003212905746, 4.25728363e-08, 0, 0, 0, 0, 0, 0, 0
]