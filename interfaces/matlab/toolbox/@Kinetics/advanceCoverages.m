function advanceCoverages(k, dt)
% Advance the surface coverages forward in time holding the bulk phase concentrations fixed.
%
% :param k:
%     Instance of class :mat:func:`Interface`
% :param dt:
%     Time interval by which the coverages should be advanced
%

kinetics_set(k.id, 5, 0, dt);
