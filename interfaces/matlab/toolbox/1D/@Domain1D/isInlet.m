function a = isInlet(d)
% :param d:
%     Instance of class :mat:func:`Domain1D`
% :return:
%     1 if the domain is an inlet, and 0 otherwise.
%

t = domainType(d);
if t == 104
  a = 1;
else
  a = 0;
end
