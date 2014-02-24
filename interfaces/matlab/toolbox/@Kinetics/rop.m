function rop = rop(a)
% Forward and reverse rates of progress.
%
% See also: :mat:func:`rop_f`, :mat:func:`rop_r`, :mat:func:`rop_net`
%
% :param a:
%     Instance of class :mat:func:`Kinetics` (or another
%     object containing an instance of that class)
%     for which forward rate constants are desired.
% :return:
%     Returns an I x 2 array of reaction rates of
%     progress, where I is the number of reactions. The first
%     column contains the forward rates of progress, and the
%     second column the reverse rates. If this function
%     is called with no output argument, a bar graph is produced.
%

f = rop_f(a);
r = rop_r(a);
rop = [f r];
if nargout == 0
    figure
    set(gcf,'Name','Rates of Progress');
    bar(rop);
    xlabel('Reaction Number');
    ylabel('Rate of Progress [kmol/m^3-s]');
    title('Rates of Progress');
    legend('Forward', 'Reverse');
end
