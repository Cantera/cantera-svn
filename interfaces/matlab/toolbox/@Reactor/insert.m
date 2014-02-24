function insert(r, gas)
% Insert an instance of class :mat:func:`Solution` into the reactor.
%
% :param r:
%     Instance of class :mat:func:`Reactor`
% :param gas:
%     Instance of class :mat:func:`Solution` to be inserted
%

r.contents = gas;
setThermoMgr(r, gas);
setKineticsMgr(r, gas);
