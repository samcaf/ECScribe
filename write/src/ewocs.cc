/**
 * @file    ewocs.cc
 *
 * @brief   Code for generating EEC and EWOC histograms in Pythia.
 */


// ---------------------------------
// Basic imports
// ---------------------------------
#include <iostream>
#include <cmath>
#include <limits>
#include <locale>
#include <fstream>
#include <sstream>
#include <string.h>
#include <vector>
#include <map>
#include <utility>
#include <stdexcept>

#include <chrono>
using namespace std::chrono;

// for including infinity as an overflow bin
#include <limits>

// ---------------------------------
// HEP imports
// ---------------------------------
#include "Pythia8/Pythia.h"
#include "fastjet/PseudoJet.hh"
#include "fastjet/ClusterSequence.hh"

// Local imports:
#include "../include/general_utils.h"
#include "../include/jet_utils.h"
#include "../include/cmdln.h"
#include "../include/pythia_cmdln.h"

#include "../include/ewoc_utils.h"

#include "../include/opendata_utils.h"


// Type definitions for EWOCs
typedef std::pair<double, double> radius_pair;

// Type definition for histograms
typedef std::vector<double> Hist;

// =====================================
// Switches, flags, and options
// =====================================
// TOGGLE: Cut on |eta| for jets in proton-X collisions
float PROTON_COLLISION_ETA_CUT = 4;

// Default EWOC options
double _DEFAULT_WEIGHT = 1;


// =====================================
// Pair observables
// =====================================
/**
* @brief: Returns the angle between a pair of pseudojets.
*
* @param: pj1, pj2  The given pseudojets.
*
* @return: double   Angle between the pseudojets (in radians).
*/
double angle(const PseudoJet pj1, const PseudoJet pj2) {
    std::vector<double> p1 = {pj1.px(), pj1.py(), pj1.pz()};
    std::vector<double> p2 = {pj2.px(), pj2.py(), pj2.pz()};

    double th = theta(p1, p2);

    if(std::isnan(th)) {
        std::string p1_str = "<" + std::to_string(pj1.px()) + " " + std::to_string(pj1.py()) + " " + std::to_string(pj1.pz()) + ">";
        std::string p2_str = "<" + std::to_string(pj2.px()) + " " + std::to_string(pj2.py()) + " " + std::to_string(pj2.pz()) + ">";

        throw std::runtime_error("Found theta = nan, from"
                   "\n\tp_1 = " + p1_str
                 + "\n\tp_2 = "  + p2_str
             );
    }

    return th;
}
double angle_squared(PseudoJet pj1, PseudoJet pj2){
    return pow(angle(pj1, pj2), 2.);
}

double deltaR(PseudoJet pj1, PseudoJet pj2){
    return pj1.delta_R(pj2);
}
double deltaR_squared(PseudoJet pj1, PseudoJet pj2){
    return pow(pj1.delta_R(pj2), 2.);
}

double formation_time(PseudoJet pj1, PseudoJet pj2){
    return std::max(pj1.e(), pj2.e())/(pj1+pj2).m2();
}



// ####################################
// Main
// ####################################
/**
* @brief: Generates events with Pythia and creates EWOC histograms.
*
* @return: int
*/
int main (int argc, char* argv[]) {
    // Printing if told to be verbose
    int verbose = cmdln_int("verbose", argc, argv, 1);
    if (verbose >= 0) std::cout << ewoc_banner << "\n\n";

    bool DEBUG = cmdln_bool("DEBUG", argc, argv, false);

    // Starting timer
    auto start = high_resolution_clock::now();

    // ---------------------------------
    // =====================================
    // Command line setup
    // =====================================
    // ---------------------------------
    // Ensuring valid command line inputs
    if (checkPythiaInputs(argc, argv) == 1) return 1;

    // ---------------------------------
    // Getting command line variables
    // ---------------------------------
    // File to which we want to write
    std::string file_prefix = cmdln_string("file_prefix",
                                           argc, argv, "",
                                           true); /* required */

    // =:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=
    // Basic Pythia Settings
    // =:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=
    // 50k e+ e=:=> hadrons events, by default
    int         n_events      = cmdln_int("n_events", argc, argv,
                                          _NEVENTS_DEFAULT);
    int         pid_1         = cmdln_int("pid_1", argc, argv,
                                          _PID_1_DEFAULT);
    int         pid_2         = cmdln_int("pid_2", argc, argv,
                                          _PID_2_DEFAULT);
    std::string outstate_str  = cmdln_string("outstate", argc, argv,
                                          _OUTSTATE_DEFAULT);


    const bool is_proton_collision = (pid_1 == 2212 and pid_2 == 2212);

    // =:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=
    // Jet Settings
    // =:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=
    std::string jet_alg               = jetalgstr_cmdln(argc, argv);
    std::string sub_alg               = subalgstr_cmdln(argc, argv);

    radius_vector_pair_t radius_pairs = radius_pairs_cmdln(argc, argv);
    std::vector<double> jet_rads      = radius_pairs.first;
    std::vector<double> sub_rads      = radius_pairs.second;

    // Recombination schemes
    RecombinationScheme jet_recomb    = jetrecomb_cmdln(argc, argv);
    RecombinationScheme sub_recomb    = subrecomb_cmdln(argc, argv);

    // Number of inclusive jets
    int n_exclusive_jets              = cmdln_int("n_exclusive_jets",
                                                  argc, argv, -1);
    // i.e. max number of jets per event to include in analysis
    // (Ordered by energy, e.g. 1 = leading jet. -1 = all jets)
    // (Default is -1, i.e. fully inclusive)
    // (does not override pt_min/pt_max options, below:)

    double      pt_min        = cmdln_double("pt_min", argc, argv,
                                             _PTMIN_DEFAULT);
    double      pt_max        = cmdln_double("pt_max", argc, argv,
                                             _PTMAX_DEFAULT);

    // Require |eta| < eta_cut, but only for proton-proton collisions
    double       eta_cut  = cmdln_double("eta_cut", argc, argv,
                                 // default depends on collision
                                 is_proton_collision ?
                                 PROTON_COLLISION_ETA_CUT
                                 : -1.0);


    // =:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=
    // EWOC Settings
    // =:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=
    // Getting functional form of pair observable
    std::string pair_obs   = cmdln_string("pair_obs", argc, argv, "", true);

    // Getting EWOC energy weight (default value 1, restricting to n1 = n2)
    double e_weight        = cmdln_double("weight", argc, argv, _DEFAULT_WEIGHT);

    // Options for using pair or contact terms (default: use both)
    bool pair_terms    = cmdln_bool("pair_terms", argc, argv, true);
    bool contact_terms = cmdln_bool("contact_terms", argc, argv, true);

    if (not(pair_terms) and not(contact_terms))
        throw std::runtime_error("Cannot create an EWOC histogram "
                "without including either pair terms or contact "
                "terms, but both were pair_terms and contact_terms "
                "were given as false.");

    // =:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=
    // Histogram Settings
    // =:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=
    // Default: Using
    //          * logarithmically spaced bins;
    //          * with minbin and maxbin in base 10;
    //          * with outflow bins.

    // Required arguments
    int   nbins   = cmdln_int("nbins", argc, argv,
                              -1, true); /* required */
    double minbin  = cmdln_double("minbin", argc, argv,
                                   0, true);
    double maxbin  = cmdln_double("maxbin", argc, argv,
                                  0, true);

    // Optional
    bool lin_bins = cmdln_bool("lin_bins", argc, argv,
                               false); /* false by default */
    std::string bin_scheme = lin_bins ? "linear" : "logarithmic";

    bool uflow = true, oflow = true;

    // -------------------------------
    // Setting up edges and centers
    // -------------------------------
    std::vector<double> bin_edges   = get_bin_edges(
                                            minbin, maxbin, nbins,
                                            uflow, oflow);
    std::vector<double> bin_centers = get_bin_centers(
                                            minbin, maxbin, nbins,
                                            uflow, oflow);


    // =:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=
    // Output Settings
    // =:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=
    // Whether to output hist in a mathematica friendly format
    bool mathematica_format = cmdln_bool("mathematica", argc, argv, false);

    // Delimiter for histogram output
    std::string HIST_DELIM;
    std::string file_ext;
    if (mathematica_format) {
        HIST_DELIM = " ";
        file_ext   = ".txt";
    } else {
        HIST_DELIM = ", ";  // python-readable
        file_ext   = ".py";
    }

    // =:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=
    // Input Settings
    // =:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=
    bool use_opendata = cmdln_bool("use_opendata", argc, argv,
                                   true);


    // =====================================
    // Output Setup
    // =====================================
    // Set up histograms
    std::vector<Hist> ewoc_hists;

    // Set up histogram output files
    std::map<radius_pair, std::string> ewoc_outfiles;

    for (size_t irad = 0; irad < jet_rads.size(); irad++) {
        // Looping over (jet, subjet) radius pairs
        double jet_rad = jet_rads[irad];
        double sub_rad = sub_rads[irad];
        radius_pair rad_pair = radius_pair(jet_rad, sub_rad);

        // Setting up histograms
        Hist hist(nbins);
        ewoc_hists.push_back(hist);

        // Setting up output files
        std::string filename = "output/ewocs/" +
            file_prefix +
            "_jet" + str_round(jet_rad, 2) +
            "_subjet" + str_round(sub_rad,2);
        filename = periods_to_hyphens(filename);
        filename += file_ext;
        // writing a header with relevant information
        write_ewocfile_header(filename, argc, argv,
                              jet_rad, sub_rad,
                              not(mathematica_format));
        // and adding them to the dict of output files
        ewoc_outfiles.insert(std::make_pair(rad_pair, filename));
    }

    // =====================================
    // Event Generation Setup
    // =====================================
    // Usual output stream (std::cout)
    std::streambuf *old = std::cout.rdbuf();
    // Declarations (muting Pythia banner)
    Pythia8::Pythia pythia;  // Declaring Pythia8
    std::stringstream pythiastream; pythiastream.str("");
    if (verbose < 3)
        // Muting Pythia banner
        std::cout.rdbuf(pythiastream.rdbuf());
    std::cout.rdbuf(old);  // Restore std::cout

    // Setting up pythia based on command line arguments
    setup_pythia_cmdln(pythia, argc, argv);

    // ---------------------------------
    // CMS Open Data
    // ---------------------------------
    std::vector<PseudoJets> cms_events(n_events);
    if (use_opendata) {
        // Reading in CMS jet events
        od::read_events(cms_events, n_events);
    }


    // =====================================
    // Analyzing events
    // =====================================
    // ---------------------------------

    // Initializing total number of jets,
    //   summed over all events
    //   (used to normalize the histogram)
    int njets_tot = 0;

    // =====================================
    // Looping over events
    // =====================================
    for (int iev = 0; iev < n_events; iev++) {
        if (not DEBUG)
            progressbar(double(iev+1)/double(n_events));

        PseudoJets particles;

        if (not use_opendata) {
            // Considering next event, if valid
            if(!pythia.next()) continue;

            // Initializing particles for this event
            particles = get_particles_pythia(pythia.event);
        }

        // ---------------------------------
        // Looping on jet definitions:
        // ---------------------------------
        for (size_t irad = 0; irad < jet_rads.size(); irad++) {
            double jet_rad = jet_rads[irad];
            double sub_rad = sub_rads[irad];

            JetDefinition jet_def = process_JetDef(jet_alg, jet_rad,
                                                   jet_recomb);
            JetDefinition sub_def = process_JetDef(sub_alg, sub_rad,
                                                   sub_recomb);

            // -#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-
            // Jet finding (with cuts)
            // -#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-
            PseudoJets good_jets;

            // -----------------------------------------
            // CMS Open Data (gives jets from the start)
            // -----------------------------------------
            if (use_opendata) {
                if (not (jet_rads.size() == 1) and
                    jet_rads[0] == 0.5)
                    throw std::invalid_argument("To use CMS "
                            "Open Data, must ask for AKT5 jets,\n"
                            "\t--jet_alg akt --jet_rad 0.5.");


                // Getting jet associated with this event
                PseudoJet jet;

                for (auto part : cms_events[iev])
                    if (part.modp() > 0)
                        jet = join(jet, part);

                if (DEBUG)
                    std::cout << jet.pt() << " " << jet.eta() << " " << jet.phi() << std::endl;

                good_jets.push_back(jet);
            }
            // -----------------------------------------
            // If using Pythia, find jets manually
            // -----------------------------------------
            else {
                // Muting FastJet banner
                std::stringstream fastjetstream; fastjetstream.str("");
                std::cout.rdbuf(fastjetstream.rdbuf());  // Redirect output
                ClusterSequence cluster_seq(particles, jet_def); // Starting fastjet
                std::cout.rdbuf(old);  // Restore std::cout

                PseudoJets all_jets;

                if (jet_rad < 1000) {
                    // If given a generic value of R,
                    // cluster the event with the given jet definition
                    all_jets = sorted_by_E(cluster_seq.inclusive_jets());
                } else {
                    // If we are given the maximum possible value of R,
                    // use the whole event as a single "jet"
                    PseudoJet full_event;
                    for (auto part : particles)
                        if (part.modp() > 0)
                            full_event = join(full_event, part);

                    all_jets.push_back(full_event);
                }

                for (size_t i = 0; i < all_jets.size(); i++) {
                    // Only working up to the Nth jet if doing exclusive analysis
                    if (n_exclusive_jets > 0 and i >= (size_t) n_exclusive_jets)
                        break;

                    PseudoJet jet = all_jets[i];

                    // Adding jets that satisfy certain criteria to good jets list
                    if (is_proton_collision) {
                        // For pp, ensuring pt_min < pt < pt_max
                        // and |eta| < eta_cut   (or no eta_cut given)
                        if (pt_min <= jet.pt() and jet.pt() <= pt_max
                                and (abs(jet.eta()) <= eta_cut or eta_cut < 0))
                            good_jets.push_back(jet);
                    } else {
                        // For other collisions, ensuring E_min < E < E_max
                        // (for now, keeping confusing notation with, e.g.,
                        //    E_min represented by `pt_min` below)
                        if (pt_min <= jet.E() and jet.E() <= pt_max)
                            good_jets.push_back(jet);
                    }
                }
            }
            // -#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-
            // Found jets!
            // -#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-

            // -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
            // Loop on jets
            // -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
            for (auto jet : good_jets) {
                // Counting total num of jets in all events for normalization
                njets_tot++;

                // Getting subjets
                PseudoJets subjets;

                ClusterSequence sub_cluster_seq(jet.constituents(), sub_def);
                if (sub_def.R() == 0) {
                    subjets = jet.constituents();
                }
                else {
                    subjets = sorted_by_pt(sub_cluster_seq.inclusive_jets());
                }

                // -:-:-:-:-:-:-:-:-:-:-:-:-:-:-:-:-:-:-
                // Loop on subjet pairs within the jet
                // -:-:-:-:-:-:-:-:-:-:-:-:-:-:-:-:-:-:-
                for (size_t isub=0; isub < subjets.size(); isub++) {
                    PseudoJet subjet1 = subjets[isub];

                    for (size_t jsub=isub; jsub < subjets.size(); jsub++) {
                        PseudoJet subjet2 = subjets[jsub];

                        // -/-/-/-/-/-/-/-/-
                        // Weight
                        // -/-/-/-/-/-/-/-/-
                        // Initialize number of permutations of subjet pair
                        int degeneracy;

                        // Energy weight of the subjet pair
                        float weight = subjet1.e()*subjet2.e()/pow(jet.e(), 2.);
                        weight = pow(weight, e_weight);


                        // -/-/-/-/-/-/-/-/-
                        // Histogram Value
                        // -/-/-/-/-/-/-/-/-
                        // Pairwise observable/value of EWOC histogram entry
                        double val;

                        // Getting observable values
                        if (jsub == isub) {
                            // Contact terms
                            if (contact_terms)
                                degeneracy = 1;
                            else
                                degeneracy = 0;

                            // Mass
                            if (str_eq(pair_obs, "mass")
                                    or str_eq(pair_obs, "m")) {
                                val = subjet1.m();
                            }
                            // Mass-squared
                            else if (str_eq(pair_obs, "mass_squared")
                                    or str_eq(pair_obs, "mass2")
                                    or str_eq(pair_obs, "m2")) {
                                val = subjet1.m2();
                            }
                            // EECs:
                            else if (str_eq(pair_obs, "theta")
                                    or str_eq(pair_obs, "theta2")
                                    or str_eq(pair_obs, "deltaR")
                                    or str_eq(pair_obs, "deltaR2"))  {
                                val = 0;
                            }
                            // Formation time
                            else if (str_eq(pair_obs, "formtime") or
                                     str_eq(pair_obs, "formation_time") or
                                     str_eq(pair_obs, "tau")) {
                                val = std::numeric_limits<double>::infinity();
                            }
                            else
                                throw std::runtime_error("Invalid pair_obs " + pair_obs);
                        } else {
                            // Pair terms
                            if (pair_terms)
                                degeneracy = 2;
                            else
                                degeneracy = 0;

                            // Mass
                            if (str_eq(pair_obs, "mass")) {
                                val = (subjet1+subjet2).m();
                            }
                            // Mass-squared
                            else if (str_eq(pair_obs, "mass_squared")
                                    or str_eq(pair_obs, "mass2")
                                    or str_eq(pair_obs, "m2")) {
                                val = (subjet1+subjet2).m2();
                            }
                            // e+e- EECs
                            else if (str_eq(pair_obs, "theta")) {
                                val = angle(subjet1, subjet2);
                            }
                            else if (str_eq(pair_obs, "theta2")) {
                                val = angle_squared(subjet1, subjet2);
                            }
                            // pp EECs
                            else if (str_eq(pair_obs, "deltaR")) {
                                val = deltaR(subjet1, subjet2);
                            }
                            else if (str_eq(pair_obs, "deltaR2")) {
                                val = deltaR_squared(subjet1, subjet2);
                            }
                            // Formation time
                            else if (str_eq(pair_obs, "formtime") or
                                     str_eq(pair_obs, "formation_time") or
                                     str_eq(pair_obs, "tau")) {
                                val = formation_time(subjet1, subjet2);
                            }
                            else
                                throw std::runtime_error("Invalid pair_obs " + pair_obs);
                        }

                        // -/-/-/-/-/-/-/-/-
                        // Filling Histogram
                        // -/-/-/-/-/-/-/-/-
                        // Getting bin associated with given val
                        // (default: log-spaced bins; using outflow bins)
                        int ibin = bin_position(val, minbin, maxbin,
                                                nbins, bin_scheme,
                                                uflow, oflow);

                        // Fill histogram with the weight
                        ewoc_hists[irad][ibin] += degeneracy*weight;
                    }
                } // end subjet pair loop
                // -:-:-:-:-:-:-:-:-:-:-:-:-:-:-:-:-:-:-
            } // end jet loop
            // -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
        } // end jet defn loop
        // ---------------------------------
    } // end event loop
    // =====================================

    // -----------------------------------
    // Writing histograms to output files
    // -----------------------------------
    for (size_t irad = 0; irad < jet_rads.size(); irad++) {
        double jet_rad = jet_rads[irad];
        double sub_rad = sub_rads[irad];
        radius_pair rad_pair = radius_pair(jet_rad, sub_rad);

        // Getting histogram
        Hist hist = ewoc_hists[irad];

        // Opening histogram file
        std::string filename = ewoc_outfiles[rad_pair];
        std::fstream outfile;
        outfile.open(filename, std::ios_base::in |
                               std::ios_base::out |
                               std::ios_base::app);

        // Checking for existence
        if (!outfile.is_open()) {
            std::stringstream errMsg;
            errMsg << "File for EWOC output was expected "
                   << "to be open, but was not open.\n\n"
                   << "It is possible the file was unable to "
                   << "be created at the desired location:\n\n\t"
                   << "filename = " << filename << "\n\n"
                   << "Is the filename an absolute path? If not, "
                   << "that might be the problem.";
            throw std::runtime_error(errMsg.str().c_str());
        }

        // -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
        // Writing histograms to file
        // -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
        // -:-:-:-:-:-:-:-:-:-:-:-:
        // bin edges
        // -:-:-:-:-:-:-:-:-:-:-:-:
        if (not(mathematica_format)) outfile << "bin_edges = [\n";
        else outfile << "(* bin_edges *)\n";

        for (int ibin = 0; ibin < nbins+1; ibin++) {
            double edge = lin_bins ? bin_edges[ibin]
                                   : pow(10, bin_edges[ibin]);
            outfile << edge;

            if (ibin < nbins) outfile << HIST_DELIM;
            else outfile << std::endl;
        }
        outfile << bin_edges[nbins] << "\n";

        if (not(mathematica_format)) outfile << "]\n\n";

        // -:-:-:-:-:-:-:-:-:-:-:-:
        // bin centers (x_values)
        // -:-:-:-:-:-:-:-:-:-:-:-:
        if (not(mathematica_format)) outfile << "xs = [\n";
        else outfile << "\n(* xs *)\n";

        for (int ibin = 0; ibin < nbins; ibin++) {
            double val = lin_bins ? bin_centers[ibin]
                                  : pow(10, bin_centers[ibin]);
            outfile << val;

            if (ibin < nbins-1) outfile << HIST_DELIM;
            else outfile << std::endl;
        }
        if (not(mathematica_format)) outfile << "]\n\n";

        // -:-:-:-:-:-:-:-:-:-:-:-:
        // y values (histogram)
        // -:-:-:-:-:-:-:-:-:-:-:-:
        if (not(mathematica_format)) outfile << "ys = [\n";
        else outfile << "\n(* ys *)\n";

        for (int ibin = 0; ibin < nbins; ibin++) {
            // Normalizing
            double dX;

            if (lin_bins)
                dX = bin_edges[ibin+1] - bin_edges[ibin];
            else
                dX = pow(10, bin_edges[ibin+1])
                     - pow(10, bin_edges[ibin]);

            // Storing
            double histval = hist[ibin]/(dX * njets_tot);
            outfile << histval;

            if (ibin < nbins-1)
                outfile << HIST_DELIM;
            else
                outfile << std::endl;
        }
        if (not(mathematica_format)) outfile << "]";

        // Closing file
        outfile.close();
    }

    // ---------------------------------
    // Verifying successful run
    // ---------------------------------
    if (verbose >= 0) {
        std::cout << "# ==+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+= #\n"
                  << "# Complete!\n"
                  << "# ==+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+= #\n";

        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop-start);
        std::cout << "Analyzed and saved data from "
                  << std::to_string(n_events)
                  << " events in "
                  << std::to_string(float(duration.count())/pow(10, 6))
                  << " seconds.\n";
    }

    return 0;
}
