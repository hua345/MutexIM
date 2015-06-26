var express = require('express');
var router = express.Router();

/* GET users listing. */
router.get('/', function(req, res, next) {
 res.render('login', { title: 'Express' });
})
.post('/',function(req, res){
	var user = {
		username:'admin',
		password:'123456'
	}
	if(req.body.username == user.username && req.body.password == user.password)
	{
		req.session.username = user.username;
		res.redirect('/home');
	}
	else
	{
		res.redirect('/login');
	}
});;

module.exports = router;