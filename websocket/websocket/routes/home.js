var express = require('express');
var router = express.Router();

router.get('/', function(req, res){
	var user = {
		username:'admin',
		password:'123456'
	}
	res.render('home', {'title': 'Home', 'user':user});
});

module.exports = router;