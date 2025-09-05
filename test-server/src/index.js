// Author:  Corey Ferguson
// Date:    2025 September 2
// File:    index.js
// Project: fetch-json
//

const { GreetingService } = require('./service/greeting.service'),
    { LoggerService: Logger } = require('./service/logger.service'),
    { OpenAPIBackend } = require('openapi-backend'),
    { RequestService: Request } = require('./service/request.service'),
    cors = require("cors"),
    express = require("express"),
    path = require("path");

// Non-Member Fields

const PORT = 8081;

// Non-Member Functions

const main = () => {
    const api = new OpenAPIBackend({
        definition: path.join(__dirname, '/api-doc.yaml')
    })

    api.init();

    const handleError = (err, req, res) => {
        err.status ||= 500;

        const result = {
            message: err.message ?? "Unknown Error", status: err.status
        };

        Logger.error('ERROR:', { ...err, ...result });

        res.status(err.status).send(result);  
    }, 
        greeting = new GreetingService();

    api.register({
        validationFail: (c, req, res) => {
            Request.receive(req.url, req.body);

            handleError({
                ...c.validation.errors[0],
                status: 400
            }, req, res);
        },
        greeting: (c, req, res) => {
            Request.receive(req.url, req.body);

            res.json(greeting.create(req.body));   
        },
        ping: (c, req, res) => res.send({ message: "Hello, world!"}),
        notFound: (c, req, res) => res.status(404).json(`Cannot ${req.method} ${req.url}`)
    })

    const app = express();

    app.use(cors({
        origin: "*"
    }));
    app.use(express.json());
    app.use((req, res) => api.handleRequest(req, req, res));

    app.listen(PORT, () => console.log(`Server listening on port ${PORT}...`));
};

// Entry point

main();