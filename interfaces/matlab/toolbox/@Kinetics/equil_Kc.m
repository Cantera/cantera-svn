function kc = equil_Kc(a)
% Equilibrium constants for all reactions
%
% :param a:
%     Instance of class :mat:func:`Kinetics` (or another
%     object containing an instance of that class)
%     for which equilibrium constants are desired.
% :return:
%     Returns a column vector of the equilibrium constants
%     for all reactions. The vector has an entry for every
%     reaction, whether reversible or not, but non-zero values
%     occur only for the reversible reactions. If the output is
%     not assigned to a variable, a bar graph is produced instead.
%

kc = kinetics_get(a.id,14,0);
if nargout == 0
    figure
    set(gcf,'Name','Equilibrium Constants')
    bar(log10(kc))
    xlabel('Reaction Number')
    ylabel('log_{10} Kc [kmol, m, s]')
    title('Equilibrium Constants Kc')
end
