function q = rop_f(a)
% Forward rates of progress for all reactions.
%
%    See also: :mat:func:`rop_r`, :mat:func:`rop_net`, :mat:func:`rop`
%
% :param a:
%     Instance of class :mat:func:`Kinetics` (or another
%     object containing an instance of that class)
%     for which forward rate constants are desired.
% :return:
%     Returns a column vector of the forward rates of progress
%     for all reactions. If this function
%     is called with no output argument, a bar graph is produced.
%

q = kinetics_get(a.id,11,0);
if nargout == 0
    figure
    set(gcf,'Name','Rates of Progress')
    bar(q)
    xlabel('Reaction Number')
    ylabel('Forward Rate of Progress [kmol/m^3]')
    title('Forward Rates of Progress')
end
