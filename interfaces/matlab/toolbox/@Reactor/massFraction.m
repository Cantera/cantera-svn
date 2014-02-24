function y = massFraction(r, species)
% Mass fraction of species ``species`` in reactor ``r``.
%
% :param r:
%     Instance of class :mat:func:`Reactor`
% :param species:
%     String or the one-based integer index of the species
% :return:
%     The mass fraction of the specifed species in the reactor at the
%     end of the last call to :mat:func:`advance` or :mat:func:`step`.
%

k = speciesIndex(r.contents, species) - 1;
y = reactormethods(30, reactor_hndl(r), k);
