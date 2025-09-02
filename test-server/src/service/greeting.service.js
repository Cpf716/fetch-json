// Author:  Corey Ferguson
// Date:    2025 September 2
// File:    greeting.service.js
// Project: fetch-json
//
// Non-Member Fields

const DAYS = [
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday"
];
    
class GreetingService {
    // Member Fields
    
    day; // number

    // Constructors

    constructor() {
        const today = new Date();

        this.day = today.getDay();

        const tomorrow = new Date();

        tomorrow.setDate(today.getDate() + 1);
        tomorrow.setHours(0);
        tomorrow.setMinutes(0);
        tomorrow.setSeconds(0);

        // Increment day at midnight
        setTimeout(() => this.incrementDay(), tomorrow.getMilliseconds() - today.getMilliseconds());
    }

    // Member Functions

    /**
     * 
     * @param {{ firstName: string; lastName?: string | null }} options 
     * @returns string
     */
    create(options) {
        const greeting = ["Happy ", DAYS[this.day], ", ", options.firstName];

        options.lastName && greeting.push(" ", options.lastName);

        greeting.push("!");

        return greeting.join("");
    }

    incrementDay() {
        this.day = this.day === 6 ? 0 : this.day + 1;

        // Increment day every 24 hours
        setTimeout(() => this.incrementDay(), 86400 * 1000)
    }
}

module.exports = { GreetingService }