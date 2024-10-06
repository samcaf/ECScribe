import numpy as np

# ==================================
# Information
# ==================================
# Function call ```./write/new_enc/integrated_twoangle --use_opendata --use_deltaR --use_pt --weights 1 1 --minbin -6 --n_events 10000 --nbins 50 --file_prefix 10k_50bins ```

# Number of events:
n_events = 10000
# Process
energy = 14000
level = "data"
pid_1, pid_2 = 2212, 2212
outstate_str = "qcd"
# Weight information:
weight = (1, 1)
# CMS 2011A Jet Dataset:
jet_alg = "anti-kt"
jet_scheme = "E?"
jet_rad = 0.5

# ==================================
# Output Histogram:
# ==================================
theta1_edges = [
	0, 1e-06, 1.33672e-06, 1.78683e-06, 2.3885e-06, 3.19276e-06, 4.26784e-06, 5.70493e-06, 7.62591e-06, 1.01937e-05, 1.36262e-05, 1.82145e-05, 2.43477e-05, 3.25462e-05, 4.35052e-05, 5.81545e-05, 7.77365e-05, 0.000103912, 0.000138902, 0.000185674, 0.000248194, 0.000331767, 0.000443481, 0.000592812, 0.000792425, 0.00105925, 0.00141593, 0.00189271, 0.00253003, 0.00338195, 0.00452073, 0.00604296, 0.00807777, 0.0107978, 0.0144336, 0.0192938, 0.0257904, 0.0344747, 0.0460831, 0.0616004, 0.0823427, 0.110069, 0.147132, 0.196675, 0.262901, 0.351426, 0.469759, 0.627938, 0.839379, 1.12202, np.inf
]

theta1_centers = [
	0, 1.15617e-06, 1.54548e-06, 2.06588e-06, 2.7615e-06, 3.69137e-06, 4.93434e-06, 6.59585e-06, 8.81683e-06, 1.17857e-05, 1.57542e-05, 2.1059e-05, 2.815e-05, 3.76288e-05, 5.02994e-05, 6.72364e-05, 8.98764e-05, 0.00012014, 0.000160594, 0.00021467, 0.000286954, 0.000383578, 0.000512738, 0.00068539, 0.000916177, 0.00122467, 0.00163705, 0.00218829, 0.00292513, 0.0039101, 0.00522672, 0.00698668, 0.00933926, 0.012484, 0.0166877, 0.0223068, 0.029818, 0.0398585, 0.0532798, 0.0712204, 0.0952019, 0.127259, 0.17011, 0.22739, 0.303957, 0.406307, 0.54312, 0.726001, 0.970463, np.inf
]

hist = [
	0.03251147768, 0, 0, 0, 0, 0, 0, 0, 0, 5.081813387e-05, 1.215418154e-05, 3.130514816e-06, 3.870176371e-07, 1.756240013e-05, 7.434949799e-06, 6.910290313e-05, 3.494625056e-05, 0.0001052902145, 7.016970046e-05, 0.0001459403847, 0.0003173207659, 0.0009960590356, 0.001588039957, 0.0066996283, 0.01147365805, 0.02052998999, 0.02994409143, 0.04705367134, 0.08157156147, 0.1347109288, 0.2071141608, 0.2948124488, 0.3685139233, 0.4302942811, 0.4788818, 0.5069433268, 0.5225751938, 0.5226207062, 0.5107966859, 0.5210694972, 0.5214805525, 0.5249010894, 0.5383897619, 0.5431598248, 0.530192092, 0.3257597406, 0.0338431767, 0.002147681847, 3.623350911e-07, 0
]