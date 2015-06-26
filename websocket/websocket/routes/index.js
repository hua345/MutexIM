var express = require('express');
var router = express.Router();

/* GET home page. */
router.get('/', function(req, res, next) {

  res.render('index', { title: 'Websocket Chat' });
});

router.get('/logout', function(req, res){
	req.session.destory();
	res.redirect('/');
});
module.exports = router;
