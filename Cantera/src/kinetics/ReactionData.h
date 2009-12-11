/**
 *  @file ReactionData.h
 *
 */
/*
 * $Author$
 * $Revision$
 * $Date$
 */

// Copyright 2001  California Institute of Technology


#ifndef CT_REACTION_DATA_H
#define CT_REACTION_DATA_H

#include "reaction_defs.h"

namespace Cantera {

  class ReactionData {
  public:
    ReactionData() {
      reactionType = ELEMENTARY_RXN;
      number = 0;
      rxn_number = 0;
      reversible = true;
      rateCoeffType = ARRHENIUS;
      falloffType = NONE;
      error = 0;
      equation = "";
      default_3b_eff = 1.0;
      global = false;
      isReversibleWithFrac = false;
      beta = 0.0;
    }
    virtual ~ReactionData(){}

    int reactionType;
    int number;
    int rxn_number;
    vector_int reactants;
    vector_int products;
    vector_fp rorder;
    vector_fp porder;
    vector_fp rstoich;
    vector_fp pstoich;
    std::vector<grouplist_t> rgroups;
    std::vector<grouplist_t> pgroups;
    std::map<int, doublereal> thirdBodyEfficiencies;

    //! True if the current reaction is reversible. False otherwise
    bool reversible;
    int rateCoeffType;
    vector_fp rateCoeffParameters;
    vector_fp auxRateCoeffParameters;
    int falloffType;
    vector_fp falloffParameters;
    int error;
    std::string equation;
    doublereal default_3b_eff;
    vector_fp cov;
    bool global;
    bool isReversibleWithFrac;
    doublereal beta;  // for electrochemical reactions
  };
}

#endif
