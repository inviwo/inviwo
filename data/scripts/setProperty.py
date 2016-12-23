import inviwopy

(inviwopy.getApp()
    .getProcessorNetwork()
    .getProcessorByIdentifier('Background')
    .getPropertyByIdentifier('switchColors')
    .pressButton())