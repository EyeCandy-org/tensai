const tensai = require('./build/Release/tensai.node');

module.exports = {
  Tensai: tensai.Tensai,
  TensaiEngine: tensai.TensaiEngine,
  Keys: tensai.Keys,
  Mouse: tensai.Mouse
};
